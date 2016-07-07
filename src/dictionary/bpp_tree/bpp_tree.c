#include "bpp_tree.h"

/*************
 * internals *
 *************/

/*
 *  algorithm:
 *	A B+tree implementation, with keys stored in internal nodes,
 *	and keys/record addresses stored in leaf nodes.  Each node is
 *	one sector in length, except the root node whose length is
 *	3 sectors.  When traversing the tree to insert a key, full
 *	children are adjusted to make room for possible new entries.
 *	Similarly, on deletion, half-full nodes are adjusted to allow for
 *	possible deleted entries.  Adjustments are first done by
 *	examining 2 nearest neighbors at the same level, and redistibuting
 *	the keys if possible.  If redistribution won't solve the problem,
 *	nodes are split/joined as needed.  Typically, a node is 3/4 full.
 *	On insertion, if 3 nodes are full, they are split into 4 nodes,
 *	each 3/4 full.  On deletion, if 3 nodes are 1/2 full, they are
 *	joined to create 2 nodes 3/4 full.
 *
 *	A LRR (least-recently-read) buffering scheme for nodes is used to
 *	simplify storage management, and, assuming some locality of reference,
 *	improve performance.
 *
 *	To simplify matters, both internal nodes and leafs contain the
 *	same fields.
 *
 */

/* macros for addressing fields */

/* primitives */
#define bAdr(p)		*(bAdrType *) (p)
#define eAdr(p)		*(eAdrType *) (p)

/* based on k = &[key,rec,childGE] */
#define childLT(k)	bAdr((char *) k - sizeof(bAdrType))
#define key(k)		(k)
#define rec(k)		eAdr((char *) (k) + h->keySize)
#define childGE(k)	bAdr((char *) (k) + h->keySize + sizeof(eAdrType))

/* based on b = &bufType */
#define leaf(b)		b->p->leaf
#define ct(b)		b->p->ct
#define next(b)		b->p->next
#define prev(b)		b->p->prev
#define fkey(b)		& b->p->fkey
#define lkey(b)		(fkey(b) + ks((ct(b) - 1)))
#define p(b)		(char *) (b->p)

/* shortcuts */
#define ks(ct)		((ct) * h->ks)

typedef char keyType;	/* keys entries are treated as char arrays */

typedef struct {
	char		leaf;			/* first bit = 1 if leaf */
	uint16_t	ct;				/* count of keys present */
	bAdrType	prev;			/* prev node in sequence (leaf) */
	bAdrType	next;			/* next node in sequence (leaf) */
	bAdrType	childLT;		/* child LT first key */
	/* ct occurrences of [key,rec,childGE] */
	keyType		fkey;			/* first occurrence */
} nodeType;

typedef struct bufTypeTag {
	/* location of node */
	struct bufTypeTag	*next;	/* next */
	struct bufTypeTag	*prev;	/* previous */
	bAdrType			adr;	/* on disk */
	nodeType			*p;		/* in memory */
	bpp_bool_t			valid;			/* true if buffer contents valid */
	bpp_bool_t			modified;		/* true if buffer modified */
} bufType;

/* one node for each open handle */
typedef struct hNodeTag {
	file_handle_t	fp;			/* idx file */
	int				keySize;	/* key length */
	bpp_bool_t		dupKeys;	/* true if duplicate keys */
	int				sectorSize;	/* block size for idx records */
	bCompType		comp;		/* pointer to compare routine */
	bufType			root;		/* root of b-tree, room for 3 sets */
	bufType			bufList;	/* head of buf list */
	void			*malloc1;	/* malloc'd resources */
	void			*malloc2;	/* malloc'd resources */
	bufType			gbuf;		/* gather buffer, room for 3 sets */
	bufType			*curBuf;	/* current location */
	keyType			*curKey;	/* current key in current node */
	unsigned int	maxCt;		/* minimum # keys in node */
	int				ks;			/* sizeof key entry */
	bAdrType		nextFreeAdr;/* next free b-tree record address */
} hNode;

#define error(rc) lineError(__LINE__, rc)

static bErrType
lineError(
	int			lineno,
	bErrType	rc
) {
	if ((rc == bErrIO) || (rc == bErrMemory)) {
		if (!bErrLineNo) {
			bErrLineNo = lineno;
		}
	}

	return rc;
}

static bAdrType
allocAdr(
	bHandleType handle
) {
	hNode		*h = handle;
	bAdrType	adr;

	adr				= h->nextFreeAdr;
	h->nextFreeAdr	+= h->sectorSize;
	return adr;
}

static bErrType
flush(
	bHandleType handle,
	bufType		*buf
) {
	hNode	*h = handle;
	int		i, len;		/* number of bytes to write */

	/* flush buffer to disk */
	len = h->sectorSize;

	if (buf->adr == 0) {
		len *= 3;	/* root */
	}

	if (err_ok != ion_fwrite_at(h->fp, buf->adr, len, (ion_byte_t *) buf->p)) {
		return error(bErrIO);
	}

#if 0
	/* flush buffer to disk */
	len = 1;

	if (buf->adr == 0) {
		len = 3;/* root */
	}

	if (0 != fseek(h->fp, buf->adr, SEEK_SET)) {
		return error(bErrIO);
	}

	for (i = 0; i < len; i++) {
		if (1 != fwrite(&buf[i].p->leaf, sizeof(buf->p->leaf), 1, h->fp)) {
			return error(bErrIO);
		}

		if (1 != fwrite(&buf[i].p->ct, sizeof(buf->p->ct), 1, h->fp)) {
			return error(bErrIO);
		}

		if (1 != fwrite(&buf[i].p->prev, sizeof(buf->p->prev), 1, h->fp)) {
			return error(bErrIO);
		}

		if (1 != fwrite(&buf[i].p->next, sizeof(buf->p->next), 1, h->fp)) {
			return error(bErrIO);
		}

		if (1 != fwrite(&buf[i].p->childLT, sizeof(buf->p->childLT), 1, h->fp)) {
			return error(bErrIO);
		}

		if (1 != fwrite(&buf[i].p->fkey, sizeof(buf->p->fkey), 1, h->fp)) {
			return error(bErrIO);
		}
	}

#endif

	buf->modified = boolean_false;
	nDiskWrites++;
	return bErrOk;
}

static bErrType
flushAll(
	bHandleType handle
) {
	hNode		*h = handle;
	bErrType	rc;				/* return code */
	bufType		*buf;			/* buffer */

	if (h->root.modified) {
		if ((rc = flush(handle, &h->root)) != 0) {
			return rc;
		}
	}

	buf = h->bufList.next;

	while (buf != &h->bufList) {
		if (buf->modified) {
			if ((rc = flush(handle, buf)) != 0) {
				return rc;
			}
		}

		buf = buf->next;
	}

	return bErrOk;
}

static bErrType
assignBuf(
	bHandleType handle,
	bAdrType	adr,
	bufType		**b
) {
	hNode *h = handle;
	/* assign buf to adr */
	bufType		*buf;			/* buffer */
	bErrType	rc;				/* return code */

	if (adr == 0) {
		*b = &h->root;
		return bErrOk;
	}

	/* search for buf with matching adr */
	buf = h->bufList.next;

	while (buf->next != &h->bufList) {
		if (buf->valid && (buf->adr == adr)) {
			break;
		}

		buf = buf->next;
	}

	/* either buf points to a match, or it's last one in list (LRR) */
	if (buf->valid) {
		if (buf->adr != adr) {
			if (buf->modified) {
				if ((rc = flush(handle, buf)) != 0) {
					return rc;
				}
			}

			buf->adr	= adr;
			buf->valid	= boolean_false;
		}
	}
	else {
		buf->adr = adr;
	}

	/* remove from current position and place at front of list */
	buf->next->prev = buf->prev;
	buf->prev->next = buf->next;
	buf->next		= h->bufList.next;
	buf->prev		= &h->bufList;
	buf->next->prev = buf;
	buf->prev->next = buf;
	*b				= buf;
	return bErrOk;
}

static bErrType
writeDisk(
	bufType *buf
) {
	/* write buf to disk */
	buf->valid		= boolean_true;
	buf->modified	= boolean_true;
	return bErrOk;
}

static bErrType
readDisk(
	bHandleType handle,
	bAdrType	adr,
	bufType		**b
) {
	hNode *h = handle;
	/* read data into buf */
	int			i, len;
	bufType		*buf;			/* buffer */
	bErrType	rc;				/* return code */

	if ((rc = assignBuf(handle, adr, &buf)) != 0) {
		return rc;
	}

	if (!buf->valid) {
		len = h->sectorSize;

		if (adr == 0) {
			len *= 3;	/* root */
		}

		if (err_ok != ion_fread_at(h->fp, adr, len, (ion_byte_t *) buf->p)) {
			return error(bErrIO);
		}

		buf->modified	= boolean_false;
		buf->valid		= boolean_true;
		nDiskReads++;

#if 0
		len = 1;

		if (adr == 0) {
			len = 3;/* root */
		}

		if (0 != fseek(h->fp, buf->adr, SEEK_SET)) {
			return error(bErrIO);
		}

		for (i = 0; i < len; i++) {
			if (1 != fread(&buf[i].p->leaf, sizeof(buf->p->leaf), 1, h->fp)) {
				return error(bErrIO);
			}

			if (1 != fread(&buf[i].p->ct, sizeof(buf->p->ct), 1, h->fp)) {
				return error(bErrIO);
			}

			if (1 != fread(&buf[i].p->prev, sizeof(buf->p->prev), 1, h->fp)) {
				return error(bErrIO);
			}

			if (1 != fread(&buf[i].p->next, sizeof(buf->p->next), 1, h->fp)) {
				return error(bErrIO);
			}

			if (1 != fread(&buf[i].p->childLT, sizeof(buf->p->childLT), 1, h->fp)) {
				return error(bErrIO);
			}

			if (1 != fread(&buf[i].p->fkey, sizeof(buf->p->fkey), 1, h->fp)) {
				return error(bErrIO);
			}
		}

#endif

		buf->modified	= boolean_false;
		buf->valid		= boolean_true;
		nDiskReads++;
	}

	*b = buf;
	return bErrOk;
}

typedef enum { MODE_FIRST, MODE_MATCH, MODE_FGEQ, MODE_LLEQ } modeEnum;

static int
search(
	bHandleType handle,
	bufType		*buf,
	void		*key,
	eAdrType	rec,
	keyType		**mkey,
	modeEnum	mode
) {
	/*
	 * input:
	 *   p					  pointer to node
	 *   key					key to find
	 *   rec					record address (dupkey only)
	 * output:
	 *   k					  pointer to keyType info
	 * returns:
	 *   CC_EQ				  key = mkey
	 *   CC_LT				  key < mkey
	 *   CC_GT				  key > mkey
	 */
	hNode		*h = handle;
	int			cc;				/* condition code */
	int			m;				/* midpoint of search */
	int			lb;				/* lower-bound of binary search */
	int			ub;				/* upper-bound of binary search */
	bpp_bool_t	foundDup;				/* true if found a duplicate key */

	/* scan current node for key using binary search */
	foundDup	= boolean_false;
	lb			= 0;
	ub			= ct(buf) - 1;

	while (lb <= ub) {
		m		= (lb + ub) / 2;
		*mkey	= fkey(buf) + ks(m);
		cc		= h->comp((ion_key_t) key, (ion_key_t) key(*mkey), (ion_key_size_t) (h->keySize));

		if ((cc < 0) || ((cc == 0) && (MODE_FGEQ == mode))) {
			/* key less than key[m] */
			ub = m - 1;
		}
		else if ((cc > 0) || ((cc == 0) && (MODE_LLEQ == mode))) {
			/* key greater than key[m] */
			lb = m + 1;
		}
		else {
			/* keys match */
			if (h->dupKeys) {
				switch (mode) {
					case MODE_FIRST:
						/* backtrack to first key */
						ub			= m - 1;
						foundDup	= boolean_true;
						break;

					case MODE_MATCH:

						/* rec's must also match */
						if (rec < rec(*mkey)) {
							ub	= m - 1;
							cc	= CC_LT;
						}
						else if (rec > rec(*mkey)) {
							lb	= m + 1;
							cc	= CC_GT;
						}
						else {
							return CC_EQ;
						}

						break;

					case MODE_FGEQ:
					case MODE_LLEQ:	/* nop */
						break;
				}
			}
			else {
				return cc;
			}
		}
	}

	if (ct(buf) == 0) {
		/* empty list */
		*mkey = fkey(buf);
		return CC_LT;
	}

	if (h->dupKeys && (mode == MODE_FIRST) && foundDup) {
		/* next key is first key in set of duplicates */
		*mkey += ks(1);
		return CC_EQ;
	}

	if (MODE_LLEQ == mode) {
		*mkey	= fkey(buf) + ks(ub + 1);
		cc		= h->comp((ion_key_t) key, (ion_key_t) key(*mkey), (ion_key_size_t) (h->keySize));

		if ((ub == ct(buf) - 1) || ((ub != -1) && (cc <= 0))) {
			*mkey	= fkey(buf) + ks(ub);
			cc		= h->comp((ion_key_t) key, (ion_key_t) key(*mkey), (ion_key_size_t) (h->keySize));
		}

		return cc;
	}

	if (MODE_FGEQ == mode) {
		*mkey	= fkey(buf) + ks(lb);
		cc		= h->comp((ion_key_t) key, (ion_key_t) key(*mkey), (ion_key_size_t) (h->keySize));

		if ((lb < ct(buf) - 1) && (cc < 0)) {
			*mkey	= fkey(buf) + ks(lb + 1);
			cc		= h->comp((ion_key_t) key, (ion_key_t) key(*mkey), (ion_key_size_t) (h->keySize));
		}

		return cc;
	}

	/* didn't find key */
	return cc;
}

static bErrType
scatterRoot(
	bHandleType handle
) {
	hNode	*h = handle;
	bufType *gbuf;
	bufType *root;

	/* scatter gbuf to root */

	root				= &h->root;
	gbuf				= &h->gbuf;
	memcpy(fkey(root), fkey(gbuf), ks(ct(gbuf)));
	childLT(fkey(root)) = childLT(fkey(gbuf));
	ct(root)			= ct(gbuf);
	leaf(root)			= leaf(gbuf);
	return bErrOk;
}

static bErrType
scatter(
	bHandleType handle,
	bufType		*pbuf,
	keyType		*pkey,
	int			is,
	bufType		**tmp
) {
	hNode		*h = handle;
	bufType		*gbuf;			/* gather buf */
	keyType		*gkey;			/* gather buf key */
	bErrType	rc;				/* return code */
	int			iu;				/* number of tmp's used */
	int			k0Min;			/* min #keys that can be mapped to tmp[0] */
	int			knMin;			/* min #keys that can be mapped to tmp[1..3] */
	int			k0Max;			/* max #keys that can be mapped to tmp[0] */
	int			knMax;			/* max #keys that can be mapped to tmp[1..3] */
	int			sw;				/* shift width */
	int			len;			/* length of remainder of buf */
	int			base;			/* base count distributed to tmps */
	int			extra;			/* extra counts */
	int			ct;
	int			i;

	/*
	 * input:
	 *   pbuf				   parent buffer of gathered keys
	 *   pkey				   where we insert a key if needed in parent
	 *   is					 number of supplied tmps
	 *   tmp					array of tmp's to be used for scattering
	 * output:
	 *   tmp					array of tmp's used for scattering
	 */

	/* scatter gbuf to tmps, placing 3/4 max in each tmp */

	gbuf	= &h->gbuf;
	gkey	= fkey(gbuf);
	ct		= ct(gbuf);

	/****************************************
	 * determine number of tmps to use (iu) *
	 ****************************************/
	iu		= is;

	/* determine limits */
	if (leaf(gbuf)) {
		/* minus 1 to allow for insertion */
		k0Max	= h->maxCt - 1;
		knMax	= h->maxCt - 1;
		/* plus 1 to allow for deletion */
		k0Min	= (h->maxCt / 2) + 1;
		knMin	= (h->maxCt / 2) + 1;
	}
	else {
		/* can hold an extra gbuf key as it's translated to a LT pointer */
		k0Max	= h->maxCt - 1;
		knMax	= h->maxCt;
		k0Min	= (h->maxCt / 2) + 1;
		knMin	= ((h->maxCt + 1) / 2) + 1;
	}

	/* calculate iu, number of tmps to use */
	while (1) {
		if ((iu == 0) || (ct > (k0Max + (iu - 1) * knMax))) {
			/* add a buffer */
			if ((rc = assignBuf(handle, allocAdr(handle), &tmp[iu])) != 0) {
				return rc;
			}

			/* update sequential links */
			if (leaf(gbuf)) {
				/* adjust sequential links */
				if (iu == 0) {
					/* no tmps supplied when splitting root for first time */
					prev(tmp[0])	= 0;
					next(tmp[0])	= 0;
				}
				else {
					prev(tmp[iu])		= tmp[iu - 1]->adr;
					next(tmp[iu])		= next(tmp[iu - 1]);
					next(tmp[iu - 1])	= tmp[iu]->adr;
				}
			}

			iu++;
			nNodesIns++;
		}
		else if ((iu > 1) && (ct < (k0Min + (iu - 1) * knMin))) {
			/* del a buffer */
			iu--;

			/* adjust sequential links */
			if (leaf(gbuf) && tmp[iu - 1]->adr) {
				next(tmp[iu - 1]) = next(tmp[iu]);
			}

			next(tmp[iu - 1]) = next(tmp[iu]);
			nNodesDel++;
		}
		else {
			break;
		}
	}

	/* establish count for each tmp used */
	base	= ct / iu;
	extra	= ct % iu;

	for (i = 0; i < iu; i++) {
		int n;

		n = base;

		/* distribute extras, one at a time */
		/* don't do to 1st node, as it may be internal and can't hold it */
		if (i && extra) {
			n++;
			extra--;
		}

		ct(tmp[i]) = n;
	}

	/**************************************
	 * update sequential links and parent *
	 **************************************/
	if (iu != is) {
		/* link last node to next */
		if (leaf(gbuf) && next(tmp[iu - 1])) {
			bufType *buf;

			if ((rc = readDisk(handle, next(tmp[iu - 1]), &buf)) != 0) {
				return rc;
			}

			prev(buf) = tmp[iu - 1]->adr;

			if ((rc = writeDisk(buf)) != 0) {
				return rc;
			}
		}

		/* shift keys in parent */
		sw = ks(iu - is);

		if (sw < 0) {
			len = ks(ct(pbuf)) - (pkey - fkey(pbuf)) + sw;
			memmove(pkey, pkey - sw, len);
		}
		else {
			len = ks(ct(pbuf)) - (pkey - fkey(pbuf));
			memmove(pkey + sw, pkey, len);
		}

		/* don't count LT buffer for empty parent */
		if (ct(pbuf)) {
			ct(pbuf) += iu - is;
		}
		else {
			ct(pbuf) += iu - is - 1;
		}
	}

	/*******************************
	 * distribute keys to children *
	 *******************************/
	for (i = 0; i < iu; i++) {
		/* update LT pointer and parent nodes */
		if (leaf(gbuf)) {
			/* update LT, tmp[i] */
			childLT(fkey(tmp[i])) = 0;

			/* update parent */
			if (i == 0) {
				childLT(pkey) = tmp[i]->adr;
			}
			else {
				memcpy(pkey, gkey, ks(1));
				childGE(pkey)	= tmp[i]->adr;
				pkey			+= ks(1);
			}
		}
		else {
			if (i == 0) {
				/* update LT, tmp[0] */
				childLT(fkey(tmp[i]))	= childLT(gkey);
				/* update LT, parent */
				childLT(pkey)			= tmp[i]->adr;
			}
			else {
				/* update LT, tmp[i] */
				childLT(fkey(tmp[i]))	= childGE(gkey);
				/* update parent key */
				memcpy(pkey, gkey, ks(1));
				childGE(pkey)			= tmp[i]->adr;
				gkey					+= ks(1);
				pkey					+= ks(1);
				ct(tmp[i])--;
			}
		}

		/* install keys, tmp[i] */
		memcpy(fkey(tmp[i]), gkey, ks(ct(tmp[i])));
		leaf(tmp[i])	= leaf(gbuf);

		gkey			+= ks(ct(tmp[i]));
	}

	leaf(pbuf) = boolean_false;

	/************************
	 * write modified nodes *
	 ************************/
	if ((rc = writeDisk(pbuf)) != 0) {
		return rc;
	}

	for (i = 0; i < iu; i++) {
		if ((rc = writeDisk(tmp[i])) != 0) {
			return rc;
		}
	}

	return bErrOk;
}

static bErrType
gatherRoot(
	bHandleType handle
) {
	hNode	*h = handle;
	bufType *gbuf;
	bufType *root;

	/* gather root to gbuf */
	root		= &h->root;
	gbuf		= &h->gbuf;
	memcpy(p(gbuf), root->p, 3 * h->sectorSize);
	leaf(gbuf)	= leaf(root);
	ct(root)	= 0;
	return bErrOk;
}

static bErrType
gather(
	bHandleType handle,
	bufType		*pbuf,
	keyType		**pkey,
	bufType		**tmp
) {
	hNode		*h = handle;
	bErrType	rc;				/* return code */
	bufType		*gbuf;
	keyType		*gkey;

	/*
	 * input:
	 *   pbuf				   parent buffer
	 *   pkey				   pointer to match key in parent
	 * output:
	 *   tmp					buffers to use for scatter
	 *   pkey				   pointer to match key in parent
	 * returns:
	 *   bErrOk				 operation successful
	 * notes:
	 *   Gather 3 buffers to gbuf.  Setup for subsequent scatter by
	 *   doing the following:
	 *	 - setup tmp buffer array for scattered buffers
	 *	 - adjust pkey to point to first key of 3 buffers
	 */

	/* find 3 adjacent buffers */
	if (*pkey == lkey(pbuf)) {
		*pkey -= ks(1);
	}

	if ((rc = readDisk(handle, childLT(*pkey), &tmp[0])) != 0) {
		return rc;
	}

	if ((rc = readDisk(handle, childGE(*pkey), &tmp[1])) != 0) {
		return rc;
	}

	if ((rc = readDisk(handle, childGE(*pkey + ks(1)), &tmp[2])) != 0) {
		return rc;
	}

	/* gather nodes to gbuf */
	gbuf			= &h->gbuf;
	gkey			= fkey(gbuf);

	/* tmp[0] */
	childLT(gkey)	= childLT(fkey(tmp[0]));
	memcpy(gkey, fkey(tmp[0]), ks(ct(tmp[0])));
	gkey			+= ks(ct(tmp[0]));
	ct(gbuf)		= ct(tmp[0]);

	/* tmp[1] */
	if (!leaf(tmp[1])) {
		memcpy(gkey, *pkey, ks(1));
		childGE(gkey)	= childLT(fkey(tmp[1]));
		ct(gbuf)++;
		gkey			+= ks(1);
	}

	memcpy(gkey, fkey(tmp[1]), ks(ct(tmp[1])));
	gkey		+= ks(ct(tmp[1]));
	ct(gbuf)	+= ct(tmp[1]);

	/* tmp[2] */
	if (!leaf(tmp[2])) {
		memcpy(gkey, *pkey + ks(1), ks(1));
		childGE(gkey)	= childLT(fkey(tmp[2]));
		ct(gbuf)++;
		gkey			+= ks(1);
	}

	memcpy(gkey, fkey(tmp[2]), ks(ct(tmp[2])));
	ct(gbuf)	+= ct(tmp[2]);

	leaf(gbuf)	= leaf(tmp[0]);

	return bErrOk;
}

bErrType
bOpen(
	bOpenType	info,
	bHandleType *handle
) {
	hNode		*h;
	bErrType	rc;				/* return code */
	int			bufCt;			/* number of tmp buffers */
	bufType		*buf;			/* buffer */
	int			maxCt;			/* maximum number of keys in a node */
	bufType		*root;
	int			i;
	nodeType	*p;

	if ((info.sectorSize < sizeof(hNode)) || (info.sectorSize % 4)) {
		return bErrSectorSize;
	}

	/* determine sizes and offsets */
	/* leaf/n, prev, next, [childLT,key,rec]... childGE */
	/* ensure that there are at least 3 children/parent for gather/scatter */
	maxCt	= info.sectorSize - (sizeof(nodeType) - sizeof(keyType));
	maxCt	/= sizeof(bAdrType) + info.keySize + sizeof(eAdrType);

	if (maxCt < 6) {
		return bErrSectorSize;
	}

	/* copy parms to hNode */
	if ((h = malloc(sizeof(hNode))) == NULL) {
		return error(bErrMemory);
	}

	for (i = 0; ((unsigned int) i) < sizeof(hNode); i++) {
		((char *) h)[i] = 0;
	}

	memset(h, 0, sizeof(hNode));
	h->keySize		= info.keySize;
	h->dupKeys		= info.dupKeys;
	h->sectorSize	= info.sectorSize;
	h->comp			= info.comp;

	/* childLT, key, rec */
	h->ks			= sizeof(bAdrType) + h->keySize + sizeof(eAdrType);
	h->maxCt		= maxCt;

	/* Allocate buflist.
	 * During insert/delete, need simultaneous access to 7 buffers:
	 *  - 4 adjacent child bufs
	 *  - 1 parent buf
	 *  - 1 next sequential link
	 *  - 1 lastGE
	 */
	bufCt			= 7;

	if ((h->malloc1 = malloc(bufCt * sizeof(bufType))) == NULL) {
		return error(bErrMemory);
	}

	for (i = 0; ((unsigned int) i) < bufCt * sizeof(bufType); i++) {
		((char *) h->malloc1)[i] = 0;
	}

	buf = h->malloc1;

	/*
	 * Allocate bufs.
	 * We need space for the following:
	 *  - bufCt buffers, of size sectorSize
	 *  - 1 buffer for root, of size 3*sectorSize
	 *  - 1 buffer for gbuf, size 3*sectorsize + 2 extra keys
	 *	to allow for LT pointers in last 2 nodes when gathering 3 full nodes
	 */
	if ((h->malloc2 = malloc((bufCt + 6) * h->sectorSize + 2 * h->ks)) == NULL) {
		return error(bErrMemory);
	}

	for (i = 0; i < (bufCt + 6) * h->sectorSize + 2 * h->ks; i++) {
		((char *) h->malloc2)[i] = 0;
	}

	p				= h->malloc2;

	/* initialize buflist */
	h->bufList.next = buf;
	h->bufList.prev = buf + (bufCt - 1);

	for (i = 0; i < bufCt; i++) {
		buf->next		= buf + 1;
		buf->prev		= buf - 1;
		buf->modified	= boolean_false;
		buf->valid		= boolean_false;
		buf->p			= p;
		p				= (nodeType *) ((char *) p + h->sectorSize);
		buf++;
	}

	h->bufList.next->prev	= &h->bufList;
	h->bufList.prev->next	= &h->bufList;

	/* initialize root */
	root					= &h->root;
	root->p					= p;
	p						= (nodeType *) ((char *) p + 3 * h->sectorSize);
	h->gbuf.p				= p;/* done last to include extra 2 keys */

	h->curBuf				= NULL;
	h->curKey				= NULL;

	/* initialize root */
	if (ion_fexists(info.iName)) {
		/* open an existing database */
		h->fp = ion_fopen(info.iName);

		if ((rc = readDisk(h, 0, &root)) != 0) {
			return rc;
		}

		if (ion_fseek(h->fp, 0, ION_FILE_END)) {
			return error(bErrIO);
		}

		if ((h->nextFreeAdr = ion_ftell(h->fp)) == -1) {
			return error(bErrIO);
		}
	}

	/*TODO make this cleaner **/
#if defined(ARDUINO)
	else if (NULL != (h->fp = ion_fopen(info.iName)).file) {
#else
	else if (NULL != (h->fp = ion_fopen(info.iName))) {
#endif
		/* initialize root */
		memset(root->p, 0, 3 * h->sectorSize);
		leaf(root)		= 1;
		h->nextFreeAdr	= 3 * h->sectorSize;
	}
	else {
		/* something's wrong */
		free(h);
		return bErrFileNotOpen;
	}

	*handle = h;
	return bErrOk;
}

bErrType
bClose(
	bHandleType handle
) {
	hNode *h = handle;

	if (h == NULL) {
		return bErrOk;
	}

	/* flush idx */
/*TODO: Cleanup **/
#if defined(ARDUINO)

	if (h->fp.file) {
#else

	if (h->fp) {
#endif
		flushAll(handle);
		ion_fclose(h->fp);
	}

	if (h->malloc2) {
		free(h->malloc2);
	}

	if (h->malloc1) {
		free(h->malloc1);
	}

	free(h);
	return bErrOk;
}

bErrType
bFindKey(
	bHandleType handle,
	void		*key,
	eAdrType	*rec
) {
	keyType		*mkey;			/* matched key */
	bufType		*buf;			/* buffer */
	bErrType	rc;				/* return code */

	hNode *h = handle;

	buf = &h->root;

	/* find key, and return address */
	while (1) {
		if (leaf(buf)) {
			if (search(handle, buf, key, 0, &mkey, MODE_FIRST) == 0) {
				*rec		= rec(mkey);
				h->curBuf	= buf;
				h->curKey	= mkey;
				return bErrOk;
			}
			else {
				return bErrKeyNotFound;
			}
		}
		else {
			if (search(handle, buf, key, 0, &mkey, MODE_FIRST) < 0) {
				if ((rc = readDisk(handle, childLT(mkey), &buf)) != 0) {
					return rc;
				}
			}
			else {
				if ((rc = readDisk(handle, childGE(mkey), &buf)) != 0) {
					return rc;
				}
			}
		}
	}
}

bErrType
bFindFirstGreaterOrEqual(
	bHandleType handle,
	void		*key,
	void		*mkey,
	eAdrType	*rec
) {
	keyType		*lgeqkey;			/* matched key */
	bufType		*buf;			/* buffer */
	bErrType	rc;				/* return code */
	int			cc;

	hNode *h = handle;

	buf = &h->root;

	/* find key, and return address */
	while (1) {
		if (leaf(buf)) {
			if ((cc = search(handle, buf, key, 0, &lgeqkey, MODE_LLEQ)) > 0) {
				if ((lgeqkey - fkey(buf)) / (h->ks) == (ct(buf))) {
					return bErrKeyNotFound;
				}

				lgeqkey += ks(1);
			}

			h->curBuf	= buf;
			h->curKey	= lgeqkey;
			memcpy(mkey, key(lgeqkey), h->keySize);
			*rec		= rec(lgeqkey);

			return bErrOk;
		}
		else {
			cc = search(handle, buf, key, 0, &lgeqkey, MODE_LLEQ);

			if (cc < 0) {
				if ((rc = readDisk(handle, childLT(lgeqkey), &buf)) != 0) {
					return rc;
				}
			}
			else {
				if ((rc = readDisk(handle, childGE(lgeqkey), &buf)) != 0) {
					return rc;
				}
			}
		}
	}
}

bErrType
bInsertKey(
	bHandleType handle,
	void		*key,
	eAdrType	rec
) {
	int				rc;			/* return code */
	keyType			*mkey;		/* match key */
	int				len;		/* length to shift */
	int				cc;			/* condition code */
	bufType			*buf, *root;
	bufType			*tmp[4];
	unsigned int	keyOff;
	bpp_bool_t		lastGEvalid;		/* true if GE branch taken */
	bpp_bool_t		lastLTvalid;		/* true if LT branch taken after GE branch */
	bAdrType		lastGE;		/* last childGE traversed */
	unsigned int	lastGEkey;	/* last childGE key traversed */
	int				height;		/* height of tree */

	hNode *h = handle;

	root		= &h->root;
	lastGEvalid = boolean_false;
	lastLTvalid = boolean_false;

	/* check for full root */
	if (ct(root) == 3 * h->maxCt) {
		/* gather root and scatter to 4 bufs */
		/* this increases b-tree height by 1 */
		if ((rc = gatherRoot(handle)) != 0) {
			return rc;
		}

		if ((rc = scatter(handle, root, fkey(root), 0, tmp)) != 0) {
			return rc;
		}
	}

	buf		= root;
	height	= 0;

	while (1) {
		if (leaf(buf)) {
			/* in leaf, and there' room guaranteed */

			if (height > maxHeight) {
				maxHeight = height;
			}

			/* set mkey to point to insertion point */
			switch (search(handle, buf, key, rec, &mkey, MODE_MATCH)) {
				case CC_LT:	/* key < mkey */

					if (!h->dupKeys && (0 != ct(buf)) && (h->comp((ion_key_t) key, (ion_key_t) mkey, (ion_key_size_t) (h->keySize)) == CC_EQ)) {
						return bErrDupKeys;
					}

					break;

				case CC_EQ:	/* key = mkey */
					return bErrDupKeys;
					break;

				case CC_GT:	/* key > mkey */

					if (!h->dupKeys && (h->comp((ion_key_t) key, (ion_key_t) mkey, (ion_key_size_t) (h->keySize)) == CC_EQ)) {
						return bErrDupKeys;
					}

					mkey += ks(1);
					break;
			}

			/* shift items GE key to right */
			keyOff	= mkey - fkey(buf);
			len		= ks(ct(buf)) - keyOff;

			if (len) {
				memmove(mkey + ks(1), mkey, len);
			}

			/* insert new key */
			memcpy(key(mkey), key, h->keySize);
			rec(mkey)		= rec;
			childGE(mkey)	= 0;
			ct(buf)++;

			if ((rc = writeDisk(buf)) != 0) {
				return rc;
			}

			/* if new key is first key, then fixup lastGE key */
			if (!keyOff && lastLTvalid) {
				bufType *tbuf;
				keyType *tkey;

				if ((rc = readDisk(handle, lastGE, &tbuf)) != 0) {
					return rc;
				}

				/* tkey = fkey(tbuf) + lastGEkey; */
				tkey		= fkey(tbuf);
				memcpy(key(tkey), key, h->keySize);
				rec(tkey)	= rec;

				if ((rc = writeDisk(tbuf)) != 0) {
					return rc;
				}
			}

			nKeysIns++;
			break;
		}
		else {
			/* internal node, descend to child */
			bufType *cbuf;	/* child buf */

			height++;

			/* read child */
			if ((cc = search(handle, buf, key, rec, &mkey, MODE_MATCH)) < 0) {
				if ((rc = readDisk(handle, childLT(mkey), &cbuf)) != 0) {
					return rc;
				}
			}
			else {
				if ((rc = readDisk(handle, childGE(mkey), &cbuf)) != 0) {
					return rc;
				}
			}

			/* check for room in child */
			if (ct(cbuf) == h->maxCt) {
				/* gather 3 bufs and scatter */
				if ((rc = gather(handle, buf, &mkey, tmp)) != 0) {
					return rc;
				}

				if ((rc = scatter(handle, buf, mkey, 3, tmp)) != 0) {
					return rc;
				}

				/* read child */
				if ((cc = search(handle, buf, key, rec, &mkey, MODE_MATCH)) < 0) {
					if ((rc = readDisk(handle, childLT(mkey), &cbuf)) != 0) {
						return rc;
					}
				}
				else {
					if ((rc = readDisk(handle, childGE(mkey), &cbuf)) != 0) {
						return rc;
					}
				}
			}

			if ((cc >= 0) || (mkey != fkey(buf))) {
				lastGEvalid = boolean_true;
				lastLTvalid = boolean_false;
				lastGE		= buf->adr;
				lastGEkey	= mkey - fkey(buf);

				if (cc < 0) {
					lastGEkey -= ks(1);
				}
			}
			else {
				if (lastGEvalid) {
					lastLTvalid = boolean_true;
				}
			}

			buf = cbuf;
		}
	}

	return bErrOk;
}

bErrType
bUpdateKey(
	bHandleType handle,
	void		*key,
	eAdrType	rec
) {
	int		rc;					/* return code */
	keyType *mkey;	/* match key */
	int		cc;					/* condition code */
	bufType *buf, *root;
	bufType *tmp[4];
	int		height;				/* height of tree */

	hNode *h = handle;

	root = &h->root;

	/* check for full root */
	if (ct(root) == 3 * h->maxCt) {
		/* gather root and scatter to 4 bufs */
		/* this increases b-tree height by 1 */
		if ((rc = gatherRoot(handle)) != 0) {
			return rc;
		}

		if ((rc = scatter(handle, root, fkey(root), 0, tmp)) != 0) {
			return rc;
		}
	}

	buf		= root;
	height	= 0;

	while (1) {
		if (leaf(buf)) {
			/* in leaf, and there' room guaranteed */

			if (height > maxHeight) {
				maxHeight = height;
			}

			/* set mkey to point to update point */
			switch (search(handle, buf, key, rec, &mkey, MODE_MATCH)) {
				case CC_LT:	/* key < mkey */
					return bErrKeyNotFound;
					break;

				case CC_EQ:	/* key = mkey */
					break;

				case CC_GT:	/* key > mkey */
					return bErrKeyNotFound;
					break;
			}

			/* update key */
			rec(mkey) = rec;
			break;
		}
		else {
			/* internal node, descend to child */
			bufType *cbuf;	/* child buf */

			height++;

			/* read child */
			if ((cc = search(handle, buf, key, rec, &mkey, MODE_MATCH)) < 0) {
				if ((rc = readDisk(handle, childLT(mkey), &cbuf)) != 0) {
					return rc;
				}
			}
			else {
				if ((rc = readDisk(handle, childGE(mkey), &cbuf)) != 0) {
					return rc;
				}
			}

			/* check for room in child */
			if (ct(cbuf) == h->maxCt) {
				/* gather 3 bufs and scatter */
				if ((rc = gather(handle, buf, &mkey, tmp)) != 0) {
					return rc;
				}

				if ((rc = scatter(handle, buf, mkey, 3, tmp)) != 0) {
					return rc;
				}

				/* read child */
				if ((cc = search(handle, buf, key, rec, &mkey, MODE_MATCH)) < 0) {
					if ((rc = readDisk(handle, childLT(mkey), &cbuf)) != 0) {
						return rc;
					}
				}
				else {
					if ((rc = readDisk(handle, childGE(mkey), &cbuf)) != 0) {
						return rc;
					}
				}
			}

			buf = cbuf;
		}
	}

	return bErrOk;
}

bErrType
bDeleteKey(
	bHandleType handle,
	void		*key,
	eAdrType	*rec
) {
	int				rc;			/* return code */
	keyType			*mkey;		/* match key */
	int				len;		/* length to shift */
	int				cc;			/* condition code */
	bufType			*buf;		/* buffer */
	bufType			*tmp[4];
	unsigned int	keyOff;
	bpp_bool_t		lastGEvalid;		/* true if GE branch taken */
	bpp_bool_t		lastLTvalid;		/* true if LT branch taken after GE branch */
	bAdrType		lastGE;		/* last childGE traversed */
	unsigned int	lastGEkey;	/* last childGE key traversed */
	bufType			*root;
	bufType			*gbuf;

	hNode *h = handle;

	root		= &h->root;
	gbuf		= &h->gbuf;
	lastGEvalid = boolean_false;
	lastLTvalid = boolean_false;

	buf			= root;

	while (1) {
		if (leaf(buf)) {
			/* set mkey to point to deletion point */
			if (search(handle, buf, key, *rec, &mkey, MODE_MATCH) == 0) {
				*rec = rec(mkey);
			}
			else {
				return bErrKeyNotFound;
			}

			/* shift items GT key to left */
			keyOff	= mkey - fkey(buf);
			len		= ks(ct(buf) - 1) - keyOff;

			if (len) {
				memmove(mkey, mkey + ks(1), len);
			}

			ct(buf)--;

			if ((rc = writeDisk(buf)) != 0) {
				return rc;
			}

			/* if deleted key is first key, then fixup lastGE key */
			if (!keyOff && lastLTvalid) {
				bufType *tbuf;
				keyType *tkey;

				if ((rc = readDisk(handle, lastGE, &tbuf)) != 0) {
					return rc;
				}

				tkey		= fkey(tbuf) + lastGEkey;
				memcpy(key(tkey), mkey, h->keySize);
				rec(tkey)	= rec(mkey);

				if ((rc = writeDisk(tbuf)) != 0) {
					return rc;
				}
			}

			nKeysDel++;
			break;
		}
		else {
			/* internal node, descend to child */
			bufType *cbuf;	/* child buf */

			/* read child */
			if ((cc = search(handle, buf, key, *rec, &mkey, MODE_MATCH)) < 0) {
				if ((rc = readDisk(handle, childLT(mkey), &cbuf)) != 0) {
					return rc;
				}
			}
			else {
				if ((rc = readDisk(handle, childGE(mkey), &cbuf)) != 0) {
					return rc;
				}
			}

			/* check for room to delete */
			if (ct(cbuf) == h->maxCt / 2) {
				/* gather 3 bufs and scatter */
				if ((rc = gather(handle, buf, &mkey, tmp)) != 0) {
					return rc;
				}

				/* if last 3 bufs in root, and count is low enough... */
				if ((buf == root) && (ct(root) == 2) && (ct(gbuf) < (3 * (3 * h->maxCt)) / 4)) {
					/* collapse tree by one level */
					scatterRoot(handle);
					nNodesDel += 3;
					continue;
				}

				if ((rc = scatter(handle, buf, mkey, 3, tmp)) != 0) {
					return rc;
				}

				/* read child */
				if ((cc = search(handle, buf, key, *rec, &mkey, MODE_MATCH)) < 0) {
					if ((rc = readDisk(handle, childLT(mkey), &cbuf)) != 0) {
						return rc;
					}
				}
				else {
					if ((rc = readDisk(handle, childGE(mkey), &cbuf)) != 0) {
						return rc;
					}
				}
			}

			if ((cc >= 0) || (mkey != fkey(buf))) {
				lastGEvalid = boolean_true;
				lastLTvalid = boolean_false;
				lastGE		= buf->adr;
				lastGEkey	= mkey - fkey(buf);

				if (cc < 0) {
					lastGEkey -= ks(1);
				}
			}
			else {
				if (lastGEvalid) {
					lastLTvalid = boolean_true;
				}
			}

			buf = cbuf;
		}
	}

	return bErrOk;
}

bErrType
bFindFirstKey(
	bHandleType handle,
	void		*key,
	eAdrType	*rec
) {
	bErrType	rc;				/* return code */
	bufType		*buf;			/* buffer */

	hNode *h = handle;

	buf = &h->root;

	while (!leaf(buf)) {
		if ((rc = readDisk(handle, childLT(fkey(buf)), &buf)) != 0) {
			return rc;
		}
	}

	if (ct(buf) == 0) {
		return bErrKeyNotFound;
	}

	memcpy(key, key(fkey(buf)), h->keySize);
	*rec		= rec(fkey(buf));
	h->curBuf	= buf;
	h->curKey	= fkey(buf);
	return bErrOk;
}

bErrType
bFindLastKey(
	bHandleType handle,
	void		*key,
	eAdrType	*rec
) {
	bErrType	rc;				/* return code */
	bufType		*buf;			/* buffer */

	hNode *h = handle;

	buf = &h->root;

	while (!leaf(buf)) {
		if ((rc = readDisk(handle, childGE(lkey(buf)), &buf)) != 0) {
			return rc;
		}
	}

	if (ct(buf) == 0) {
		return bErrKeyNotFound;
	}

	memcpy(key, key(lkey(buf)), h->keySize);
	*rec		= rec(lkey(buf));
	h->curBuf	= buf;
	h->curKey	= lkey(buf);
	return bErrOk;
}

bErrType
bFindNextKey(
	bHandleType handle,
	void		*key,
	eAdrType	*rec
) {
	bErrType	rc;				/* return code */
	keyType		*nkey;			/* next key */
	bufType		*buf;			/* buffer */

	hNode *h = handle;

	if ((buf = h->curBuf) == NULL) {
		return bErrKeyNotFound;
	}

	if (h->curKey == lkey(buf)) {
		/* current key is last key in leaf node */
		if (next(buf)) {
			/* fetch next set */
			if ((rc = readDisk(handle, next(buf), &buf)) != 0) {
				return rc;
			}

			nkey = fkey(buf);
		}
		else {
			/* no more sets */
			return bErrKeyNotFound;
		}
	}
	else {
		/* bump to next key */
		nkey = h->curKey + ks(1);
	}

	memcpy(key, key(nkey), h->keySize);
	*rec		= rec(nkey);
	h->curBuf	= buf;
	h->curKey	= nkey;
	return bErrOk;
}

bErrType
bFindPrevKey(
	bHandleType handle,
	void		*key,
	eAdrType	*rec
) {
	bErrType	rc;				/* return code */
	keyType		*pkey;			/* previous key */
	keyType		*fkey;			/* first key */
	bufType		*buf;			/* buffer */

	hNode *h = handle;

	if ((buf = h->curBuf) == NULL) {
		return bErrKeyNotFound;
	}

	fkey = fkey(buf);

	if (h->curKey == fkey) {
		/* current key is first key in leaf node */
		if (prev(buf)) {
			/* fetch previous set */
			if ((rc = readDisk(handle, prev(buf), &buf)) != 0) {
				return rc;
			}

			pkey = fkey(buf) + ks((ct(buf) - 1));
		}
		else {
			/* no more sets */
			return bErrKeyNotFound;
		}
	}
	else {
		/* bump to previous key */
		pkey = h->curKey - ks(1);
	}

	memcpy(key, key(pkey), h->keySize);
	*rec		= rec(pkey);
	h->curBuf	= buf;
	h->curKey	= pkey;
	return bErrOk;
}
