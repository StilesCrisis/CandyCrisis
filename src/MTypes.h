///
///  MTypes.h
///
///  Generic replacements for very basic Mac types.
///
///  John Stiles, 2002/10/14
///


#pragma once

typedef signed char MBoolean;


struct MRect
{
	short top;
	short left;
	short bottom;
	short right;	
};


struct MPoint
{
	short v;
	short h;
};


void InflateMRect( MRect* r, int dx, int dy );
void UnionMRect( const MRect* a, const MRect* b, MRect* u );
void OffsetMRect( MRect* r, int x, int y );
unsigned char MPointInMRect( const MPoint& p, const MRect* r );

