/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Common Types

Classes:
***********************************************************************/

#ifndef VCZH_PRESENTATION_GUITYPES
#define VCZH_PRESENTATION_GUITYPES

#include "../Import/Vlpp.h"
#include "../Import/VlppWorkflowLibrary.h"

namespace vl
{
	namespace presentation
	{

/***********************************************************************
Enumerations
***********************************************************************/
		
		/// <summary>
		/// Defines an alignment direction.
		/// </summary>
		enum class Alignment
		{
			/// <summary>Aligned to the left side.</summary>
			Left=0,
			/// <summary>Aligned to the top side.</summary>
			Top=0,
			/// <summary>Aligned to the center.</summary>
			Center=1,
			/// <summary>Aligned to the right side.</summary>
			Right=2,
			/// <summary>Aligned to the bottom side.</summary>
			Bottom=2,
		};

		/// <summary>Axis direction.</summary>
		enum class AxisDirection
		{
			/// <summary>X:left, Y:down.</summary>
			LeftDown,
			/// <summary>X:right, Y:down.</summary>
			RightDown,
			/// <summary>X:left, Y:up.</summary>
			LeftUp,
			/// <summary>X:right, Y:up.</summary>
			RightUp,
			/// <summary>X:down, Y:left.</summary>
			DownLeft,
			/// <summary>X:down, Y:right.</summary>
			DownRight,
			/// <summary>X:up, Y:left.</summary>
			UpLeft,
			/// <summary>X:up, Y:right.</summary>
			UpRight,
		};

/***********************************************************************
TextPos
***********************************************************************/
		
		/// <summary>
		/// Represents the position in multiple lines of text.
		/// </summary>
		struct TextPos
		{
			/// <summary>
			/// Row number.
			/// </summary>
			vint			row;
			/// <summary>
			/// Column number. If a line has 4 characters, there are 5 available column numbers(from 0 to 4) in this line.
			/// </summary>
			vint			column;

			TextPos()
				:row(0) ,column(0)
			{
			}

			TextPos(vint _row, vint _column)
				:row(_row) ,column(_column)
			{
			}

			vint Compare(const TextPos& value)const
			{
				if(row<value.row) return -1;
				if(row>value.row) return 1;
				if(column<value.column) return -1;
				if(column>value.column) return 1;
				return 0;
			}

			bool operator==(const TextPos& value)const {return Compare(value)==0;}
			bool operator!=(const TextPos& value)const {return Compare(value)!=0;}
			bool operator<(const TextPos& value)const {return Compare(value)<0;}
			bool operator<=(const TextPos& value)const {return Compare(value)<=0;}
			bool operator>(const TextPos& value)const {return Compare(value)>0;}
			bool operator>=(const TextPos& value)const {return Compare(value)>=0;}
		};

/***********************************************************************
GridPos
***********************************************************************/
		
		/// <summary>
		/// Represents the cell position in a grid.
		/// </summary>
		struct GridPos
		{
			/// <summary>
			/// Row number.
			/// </summary>
			vint			row;
			/// <summary>
			/// Column number. If a line has 4 characters, there are 5 available column numbers(from 0 to 4) in this line.
			/// </summary>
			vint			column;

			GridPos()
				:row(0) ,column(0)
			{
			}

			GridPos(vint _row, vint _column)
				:row(_row) ,column(_column)
			{
			}

			vint Compare(const GridPos& value)const
			{
				if(row<value.row) return -1;
				if(row>value.row) return 1;
				if(column<value.column) return -1;
				if(column>value.column) return 1;
				return 0;
			}

			bool operator==(const GridPos& value)const {return Compare(value)==0;}
			bool operator!=(const GridPos& value)const {return Compare(value)!=0;}
			bool operator<(const GridPos& value)const {return Compare(value)<0;}
			bool operator<=(const GridPos& value)const {return Compare(value)<=0;}
			bool operator>(const GridPos& value)const {return Compare(value)>0;}
			bool operator>=(const GridPos& value)const {return Compare(value)>=0;}
		};

/***********************************************************************
Point
***********************************************************************/
		
		/// <summary>
		/// Represents a position in a two dimensions space.
		/// </summary>
		struct Point
		{
			/// <summary>
			/// Position in x dimension.
			/// </summary>
			vint			x;
			/// <summary>
			/// Position in y dimension.
			/// </summary>
			vint			y;

			Point()
				:x(0) ,y(0)
			{
			}

			Point(vint _x, vint _y)
				:x(_x) ,y(_y)
			{
			}

			bool operator==(Point point)const
			{
				return x==point.x && y==point.y;
			}

			bool operator!=(Point point)const
			{
				return x!=point.x || y!=point.y;
			}
		};

/***********************************************************************
Size
***********************************************************************/
		
		/// <summary>
		/// Represents a size in a two dimensions space.
		/// </summary>
		struct Size
		{
			/// <summary>
			/// Size in x dimension.
			/// </summary>
			vint			x;
			/// <summary>
			/// Size in y dimension.
			/// </summary>
			vint			y;

			Size()
				:x(0) ,y(0)
			{
			}

			Size(vint _x, vint _y)
				:x(_x) ,y(_y)
			{
			}

			bool operator==(Size size)const
			{
				return x==size.x && y==size.y;
			}

			bool operator!=(Size size)const
			{
				return x!=size.x || y!=size.y;
			}
		};

/***********************************************************************
Rectangle
***********************************************************************/
		
		/// <summary>
		/// Represents a bounds in a two dimensions space.
		/// </summary>
		struct Rect
		{
			/// <summary>
			/// Left.
			/// </summary>
			vint		x1;
			/// <summary>
			/// Top.
			/// </summary>
			vint		y1;
			/// <summary>
			/// Left + Width.
			/// </summary>
			vint		x2;
			/// <summary>
			/// Top + Height.
			/// </summary>
			vint		y2;

			Rect()
				:x1(0), y1(0), x2(0), y2(0)
			{
			}

			Rect(vint _x1, vint _y1, vint _x2, vint _y2)
				:x1(_x1), y1(_y1), x2(_x2), y2(_y2)
			{
			}

			Rect(Point p, Size s)
				:x1(p.x), y1(p.y), x2(p.x+s.x), y2(p.y+s.y)
			{
			}

			bool operator==(Rect rect)const
			{
				return x1==rect.x1 && y1==rect.y1 && x2==rect.x2 && y2==rect.y2;
			}

			bool operator!=(Rect rect)const
			{
				return x1!=rect.x1 || y1!=rect.y1 || x2!=rect.x2 || y2!=rect.y2;
			}

			Point LeftTop()const
			{
				return Point(x1, y1);
			}

			Point RightBottom()const
			{
				return Point(x2, y2);
			}

			Size GetSize()const
			{
				return Size(x2-x1, y2-y1);
			}

			vint Left()const
			{
				return x1;
			}

			vint Right()const
			{
				return x2;
			}

			vint Width()const
			{
				return x2-x1;
			}

			vint Top()const
			{
				return y1;
			}

			vint Bottom()const
			{
				return y2;
			}

			vint Height()const
			{
				return y2-y1;
			}

			void Expand(vint x, vint y)
			{
				x1-=x;
				y1-=y;
				x2+=x;
				y2+=y;
			}

			void Expand(Size s)
			{
				x1-=s.x;
				y1-=s.y;
				x2+=s.x;
				y2+=s.y;
			}

			void Move(vint x, vint y)
			{
				x1+=x;
				y1+=y;
				x2+=x;
				y2+=y;
			}

			void Move(Size s)
			{
				x1+=s.x;
				y1+=s.y;
				x2+=s.x;
				y2+=s.y;
			}

			bool Contains(Point p)
			{
				return x1<=p.x && p.x<x2 && y1<=p.y && p.y<y2;
			}
		};

/***********************************************************************
2D operations
***********************************************************************/

		inline Point operator+(Point p, Size s)
		{
			return Point(p.x+s.x, p.y+s.y);
		}

		inline Point operator+(Size s, Point p)
		{
			return Point(p.x+s.x, p.y+s.y);
		}

		inline Point operator-(Point p, Size s)
		{
			return Point(p.x-s.x, p.y-s.y);
		}

		inline Size operator-(Point p1, Point p2)
		{
			return Size(p1.x-p2.x, p1.y-p2.y);
		}

		inline Size operator+(Size s1, Size s2)
		{
			return Size(s1.x+s2.x, s1.y+s2.y);
		}

		inline Size operator-(Size s1, Size s2)
		{
			return Size(s1.x-s2.x, s1.y-s2.y);
		}

		inline Size operator*(Size s, vint i)
		{
			return Size(s.x*i, s.y*i);
		}

		inline Size operator/(Size s, vint i)
		{
			return Size(s.x/i, s.y/i);
		}

		inline Point operator+=(Point& s1, Size s2)
		{
			s1.x+=s2.x;
			s1.y+=s2.y;
			return s1;
		}

		inline Point operator-=(Point& s1, Size s2)
		{
			s1.x-=s2.x;
			s1.y-=s2.y;
			return s1;
		}

		inline Size operator+=(Size& s1, Size s2)
		{
			s1.x+=s2.x;
			s1.y+=s2.y;
			return s1;
		}

		inline Size operator-=(Size& s1, Size s2)
		{
			s1.x-=s2.x;
			s1.y-=s2.y;
			return s1;
		}

/***********************************************************************
Color
***********************************************************************/
		
		/// <summary>Represents a 32bit RGBA color. Values of separate components can be accessed using fields "r", "g", "b" and "a".</summary>
		struct Color
		{
			union
			{
				struct
				{
					unsigned char r;
					unsigned char g;
					unsigned char b;
					unsigned char a;
				};
				vuint32_t value;
			};

			Color()
				:r(0), g(0), b(0), a(255)
			{
			}

			Color(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a=255)
				:r(_r), g(_g), b(_b), a(_a)
			{
			}

			vint Compare(Color color)const
			{
				return value-color.value;
			}

			static Color Parse(const WString& value)
			{
				const wchar_t* code=L"0123456789ABCDEF";
				if((value.Length()==7 || value.Length()==9) && value[0]==L'#')
				{
					vint index[8]={15, 15, 15, 15, 15, 15, 15, 15};
					for(vint i=0;i<value.Length()-1;i++)
					{
						index[i]=wcschr(code, value[i+1])-code;
						if(index[i]<0 || index[i]>15)
						{
							return Color();
						}
					}

					Color c;
					c.r=(unsigned char)(index[0]*16+index[1]);
					c.g=(unsigned char)(index[2]*16+index[3]);
					c.b=(unsigned char)(index[4]*16+index[5]);
					c.a=(unsigned char)(index[6]*16+index[7]);
					return c;
				}
				return Color();
			}

			WString ToString()const
			{
				const wchar_t* code=L"0123456789ABCDEF";
				wchar_t result[10]=L"#00000000";
				result[1]=code[r/16];
				result[2]=code[r%16];
				result[3]=code[g/16];
				result[4]=code[g%16];
				result[5]=code[b/16];
				result[6]=code[b%16];
				if(a==255)
				{
					result[7]=L'\0';
				}
				else
				{
					result[7]=code[a/16];
					result[8]=code[a%16];
				}
				return result;
			}

			bool operator==(Color color)const {return Compare(color)==0;}
			bool operator!=(Color color)const {return Compare(color)!=0;}
			bool operator<(Color color)const {return Compare(color)<0;}
			bool operator<=(Color color)const {return Compare(color)<=0;}
			bool operator>(Color color)const {return Compare(color)>0;}
			bool operator>=(Color color)const {return Compare(color)>=0;}
		};

/***********************************************************************
Margin
***********************************************************************/
		
		/// <summary>
		/// Represents a margin in a two dimensions space.
		/// </summary>
		struct Margin
		{
			/// <summary>
			/// The left margin.
			/// </summary>
			vint		left;
			/// <summary>
			/// The top margin.
			/// </summary>
			vint		top;
			/// <summary>
			/// The right margin.
			/// </summary>
			vint		right;
			/// <summary>
			/// The bottom margin.
			/// </summary>
			vint		bottom;

			Margin()
				:left(0), top(0), right(0), bottom(0)
			{
			}

			Margin(vint _left, vint _top, vint _right, vint _bottom)
				:left(_left), top(_top), right(_right), bottom(_bottom)
			{
			}

			bool operator==(Margin margin)const
			{
				return left==margin.left && top==margin.top && right==margin.right && bottom==margin.bottom;
			}

			bool operator!=(Margin margin)const
			{
				return left!=margin.left || top!=margin.top || right!=margin.right || bottom!=margin.bottom;
			}
		};

/***********************************************************************
Resources
***********************************************************************/
		
		/// <summary>
		/// Represents a font configuration.
		/// </summary>
		struct FontProperties
		{
			/// <summary>
			/// Font family (or font name, usually).
			/// </summary>
			WString				fontFamily;
			/// <summary>
			/// Font size in pixel.
			/// </summary>
			vint				size;
			/// <summary>
			/// True if the font is bold.
			/// </summary>
			bool				bold;
			/// <summary>
			/// True if the font is italic.
			/// </summary>
			bool				italic;
			/// <summary>
			/// True if the font has a underline.
			/// </summary>
			bool				underline;
			/// <summary>
			/// True if the font has a strikeline.
			/// </summary>
			bool				strikeline;
			/// <summary>
			/// True if the font has anti alias rendering.
			/// </summary>
			bool				antialias;
			/// <summary>
			/// True if the font has anti alias rendering in vertical direction.
			/// </summary>
			bool				verticalAntialias;

			FontProperties()
				:size(0)
				,bold(false)
				,italic(false)
				,underline(false)
				,strikeline(false)
				,antialias(true)
				,verticalAntialias(false)
			{
			}
			
			vint Compare(const FontProperties& value)const
			{
				vint result=0;
				
				result=WString::Compare(fontFamily, value.fontFamily);
				if(result!=0) return result;

				result=size-value.size;
				if(result!=0) return result;

				result=(vint)bold-(vint)value.bold;
				if(result!=0) return result;

				result=(vint)italic-(vint)value.italic;
				if(result!=0) return result;

				result=(vint)underline-(vint)value.underline;
				if(result!=0) return result;

				result=(vint)strikeline-(vint)value.strikeline;
				if(result!=0) return result;

				result=(vint)antialias-(vint)value.antialias;
				if(result!=0) return result;

				return 0;
			}

			bool operator==(const FontProperties& value)const {return Compare(value)==0;}
			bool operator!=(const FontProperties& value)const {return Compare(value)!=0;}
			bool operator<(const FontProperties& value)const {return Compare(value)<0;}
			bool operator<=(const FontProperties& value)const {return Compare(value)<=0;}
			bool operator>(const FontProperties& value)const {return Compare(value)>0;}
			bool operator>=(const FontProperties& value)const {return Compare(value)>=0;}
		};
	}
}

#endif