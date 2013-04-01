#if 0
ClipRegion *reg_AllocBitmapRegion(UINT8 *bitmap, INT32 width,	INT32 height)
{
	MWRECT		rect;
	MWIMAGEBITS	bits = bitmap[0];
	MWCLIPREGION *	rgn;
	int 		x, y, inrect = 0, bitnum = 0, i = 0;

	if(!(rgn = GdAllocRegion()))
		return NULL;

	/* Return an empty region if the bitmap is empty. */
	if(!bitmap || !width || !height)
		return rgn;

	for(y = 0; y < height; y++) {
		rect.top = y;
		rect.bottom = y + 1;
		for(x = 0; x < width; x++) {
			if(!bitnum--) {
				bitnum = 15;		/* FIXME no hardcoded bits */
				bits = bitmap[i++];
			}
			if(bits & 0x8000) {
				if(!inrect) {
					inrect = 1;
					rect.left = x;
				}
			} else {
				if(inrect) {
					inrect = 0;
					rect.right = x;
					GdUnionRectWithRegion(&rect, rgn);
				}
			}
			bits <<= 1;
		}
		if(inrect) {
			rect.right = x;
			GdUnionRectWithRegion(&rect, rgn);
		}
		inrect = 0;
		bitnum = 0;
	}

	return rgn;
}
#endif
