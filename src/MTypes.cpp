///
///  MTypes.c
///
///  Generic replacements for very basic Mac types.
///
///  John Stiles, 2002/10/14
///


#include "stdafx.h"
#include "MTypes.h"
#include <algorithm>

using std::min;
using std::max;


void UnionMRect( const MRect* a, const MRect* b, MRect* u )
{
	u->top    = min<>( a->top, b->top );
	u->left   = min<>( a->left, b->left );
	u->bottom = max<>( a->bottom, b->bottom );
	u->right  = max<>( a->right, b->right );
}


void OffsetMRect( MRect* r, int x, int y )
{
	r->top += y;
	r->left += x;
	r->bottom += y;
	r->right += x;
}


void InflateMRect( MRect* r, int dx, int dy )
{
	r->top    -= dy;
	r->bottom += dy;
	r->left   -= dx;
	r->right  += dx;
}


unsigned char MPointInMRect( const MPoint& p, const MRect* r )
{
	return (p.h >= r->left) &&
	       (p.h <  r->right) &&
	       (p.v >= r->top) &&
	       (p.v <  r->bottom);
}
