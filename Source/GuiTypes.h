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
		using namespace reflection;

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
Coordinate
***********************************************************************/

		/// <summary>
		/// Represents a position in the local window coordinate space, which is DPI awared.
		/// </summary>
		using GuiCoordinate = vint;
		
		/// <summary>
		/// Represents a position in the global screen coordinate space.
		/// </summary>
		struct NativeCoordinate
		{
			vint				value;

			NativeCoordinate() :value(0) {}
			NativeCoordinate(vint _value) :value(_value) {}
			NativeCoordinate(const NativeCoordinate& _value) = default;
			NativeCoordinate(NativeCoordinate&& _value) = default;
			NativeCoordinate& operator=(const NativeCoordinate& _value) = default;
			NativeCoordinate& operator=(NativeCoordinate&& _value) = default;

			inline bool operator==(NativeCoordinate c)const { return value == c.value; };
			inline bool operator!=(NativeCoordinate c)const { return value != c.value; };
			inline bool operator<(NativeCoordinate c)const { return value < c.value; };
			inline bool operator<=(NativeCoordinate c)const { return value <= c.value; };
			inline bool operator>(NativeCoordinate c)const { return value > c.value; };
			inline bool operator>=(NativeCoordinate c)const { return value >= c.value; };

			inline NativeCoordinate operator+(NativeCoordinate c)const { return value + c.value; };
			inline NativeCoordinate operator-(NativeCoordinate c)const { return value - c.value; };
			inline NativeCoordinate operator*(NativeCoordinate c)const { return value * c.value; };
			inline NativeCoordinate operator/(NativeCoordinate c)const { return value / c.value; };

			inline NativeCoordinate& operator+=(NativeCoordinate c) { value += c.value; return *this; };
			inline NativeCoordinate& operator-=(NativeCoordinate c) { value -= c.value; return *this; };
			inline NativeCoordinate& operator*=(NativeCoordinate c) { value *= c.value; return *this; };
			inline NativeCoordinate& operator/=(NativeCoordinate c) { value /= c.value; return *this; };
		};

/***********************************************************************
Point
***********************************************************************/
		
		/// <summary>
		/// Represents a position in a two dimensions space.
		/// </summary>
		/// <typeparam name="T">Type of the coordinate.</typeparam>
		template<typename T>
		struct Point_
		{
			/// <summary>
			/// Position in x dimension.
			/// </summary>
			T				x;
			/// <summary>
			/// Position in y dimension.
			/// </summary>
			T				y;

			Point_()
				:x(0), y(0)
			{
			}

			Point_(T _x, T _y)
				:x(_x), y(_y)
			{
			}

			bool operator==(Point_<T> point)const
			{
				return x == point.x && y == point.y;
			}

			bool operator!=(Point_<T> point)const
			{
				return x != point.x || y != point.y;
			}
		};

		using Point = Point_<GuiCoordinate>;
		using NativePoint = Point_<NativeCoordinate>;

/***********************************************************************
Size
***********************************************************************/
		
		/// <summary>
		/// Represents a size in a two dimensions space.
		/// </summary>
		/// <typeparam name="T">Type of the coordinate.</typeparam>
		template<typename T>
		struct Size_
		{
			/// <summary>
			/// Size in x dimension.
			/// </summary>
			T				x;
			/// <summary>
			/// Size in y dimension.
			/// </summary>
			T				y;

			Size_()
				:x(0), y(0)
			{
			}

			Size_(T _x, T _y)
				:x(_x), y(_y)
			{
			}

			bool operator==(Size_<T> size)const
			{
				return x == size.x && y == size.y;
			}

			bool operator!=(Size_<T> size)const
			{
				return x != size.x || y != size.y;
			}
		};

		using Size = Size_<GuiCoordinate>;
		using NativeSize = Size_<NativeCoordinate>;

/***********************************************************************
Rectangle
***********************************************************************/
		
		/// <summary>
		/// Represents a bounds in a two dimensions space.
		/// </summary>
		/// <typeparam name="T">Type of the coordinate.</typeparam>
		template<typename T>
		struct Rect_
		{
			/// <summary>
			/// Left.
			/// </summary>
			T				x1;
			/// <summary>
			/// Top.
			/// </summary>
			T				y1;
			/// <summary>
			/// Left + Width.
			/// </summary>
			T				x2;
			/// <summary>
			/// Top + Height.
			/// </summary>
			T				y2;

			Rect_()
				:x1(0), y1(0), x2(0), y2(0)
			{
			}

			Rect_(T _x1, T _y1, T _x2, T _y2)
				:x1(_x1), y1(_y1), x2(_x2), y2(_y2)
			{
			}

			Rect_(Point_<T> p, Size_<T> s)
				:x1(p.x), y1(p.y), x2(p.x + s.x), y2(p.y + s.y)
			{
			}

			bool operator==(Rect_<T> rect)const
			{
				return x1 == rect.x1 && y1 == rect.y1 && x2 == rect.x2 && y2 == rect.y2;
			}

			bool operator!=(Rect_<T> rect)const
			{
				return x1 != rect.x1 || y1 != rect.y1 || x2 != rect.x2 || y2 != rect.y2;
			}

			Point_<T> LeftTop()const
			{
				return Point_<T>(x1, y1);
			}

			Point_<T> RightBottom()const
			{
				return Point_<T>(x2, y2);
			}

			Size_<T> GetSize()const
			{
				return Size_<T>(x2 - x1, y2 - y1);
			}

			T Left()const
			{
				return x1;
			}

			T Right()const
			{
				return x2;
			}

			T Width()const
			{
				return x2 - x1;
			}

			T Top()const
			{
				return y1;
			}

			T Bottom()const
			{
				return y2;
			}

			T Height()const
			{
				return y2 - y1;
			}

			void Expand(T x, T y)
			{
				x1 -= x;
				y1 -= y;
				x2 += x;
				y2 += y;
			}

			void Expand(Size_<T> s)
			{
				x1 -= s.x;
				y1 -= s.y;
				x2 += s.x;
				y2 += s.y;
			}

			void Move(T x, T y)
			{
				x1 += x;
				y1 += y;
				x2 += x;
				y2 += y;
			}

			void Move(Size_<T> s)
			{
				x1 += s.x;
				y1 += s.y;
				x2 += s.x;
				y2 += s.y;
			}

			bool Contains(Point_<T> p)
			{
				return x1 <= p.x && p.x < x2 && y1 <= p.y && p.y < y2;
			}
		};

		using Rect = Rect_<GuiCoordinate>;
		using NativeRect = Rect_<NativeCoordinate>;

/***********************************************************************
2D operations
***********************************************************************/

		template<typename T>
		inline Point_<T> operator+(Point_<T> p, Size_<T> s)
		{
			return Point_<T>(p.x + s.x, p.y + s.y);
		}

		template<typename T>
		inline Point_<T> operator+(Size_<T> s, Point_<T> p)
		{
			return Point_<T>(p.x + s.x, p.y + s.y);
		}

		template<typename T>
		inline Point_<T> operator-(Point_<T> p, Size_<T> s)
		{
			return Point_<T>(p.x - s.x, p.y - s.y);
		}

		template<typename T>
		inline Size_<T> operator-(Point_<T> p1, Point_<T> p2)
		{
			return Size_<T>(p1.x - p2.x, p1.y - p2.y);
		}

		template<typename T>
		inline Size_<T> operator+(Size_<T> s1, Size_<T> s2)
		{
			return Size_<T>(s1.x + s2.x, s1.y + s2.y);
		}

		template<typename T>
		inline Size_<T> operator-(Size_<T> s1, Size_<T> s2)
		{
			return Size_<T>(s1.x - s2.x, s1.y - s2.y);
		}

		template<typename T>
		inline Size_<T> operator*(Size_<T> s, vint i)
		{
			return Size_<T>(s.x*i, s.y*i);
		}

		template<typename T>
		inline Size_<T> operator/(Size_<T> s, vint i)
		{
			return Size_<T>(s.x / i, s.y / i);
		}

		template<typename T>
		inline Point_<T> operator+=(Point_<T>& s1, Size_<T> s2)
		{
			s1.x += s2.x;
			s1.y += s2.y;
			return s1;
		}

		template<typename T>
		inline Point_<T> operator-=(Point_<T>& s1, Size_<T> s2)
		{
			s1.x -= s2.x;
			s1.y -= s2.y;
			return s1;
		}

		template<typename T>
		inline Size_<T> operator+=(Size_<T>& s1, Size_<T> s2)
		{
			s1.x += s2.x;
			s1.y += s2.y;
			return s1;
		}

		template<typename T>
		inline Size_<T> operator-=(Size_<T>& s1, Size_<T> s2)
		{
			s1.x -= s2.x;
			s1.y -= s2.y;
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
		/// <typeparam name="T">Type of the coordinate.</typeparam>
		template<typename T>
		struct Margin_
		{
			/// <summary>
			/// The left margin.
			/// </summary>
			T			left;
			/// <summary>
			/// The top margin.
			/// </summary>
			T			top;
			/// <summary>
			/// The right margin.
			/// </summary>
			T			right;
			/// <summary>
			/// The bottom margin.
			/// </summary>
			T			bottom;

			Margin_()
				:left(0), top(0), right(0), bottom(0)
			{
			}

			Margin_(T _left, T _top, T _right, T _bottom)
				:left(_left), top(_top), right(_right), bottom(_bottom)
			{
			}

			bool operator==(Margin_<T> margin)const
			{
				return left==margin.left && top==margin.top && right==margin.right && bottom==margin.bottom;
			}

			bool operator!=(Margin_<T> margin)const
			{
				return left!=margin.left || top!=margin.top || right!=margin.right || bottom!=margin.bottom;
			}
		};

		using Margin = Margin_<GuiCoordinate>;
		using NativeMargin = Margin_<NativeCoordinate>;

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

/***********************************************************************
Keys
***********************************************************************/

#define GUI_DEFINE_KEYBOARD_CODE(ITEM)											\
/*																				\
 * Virtual Keys, Standard Set													\
 */																				\
ITEM(LBUTTON,             0x01)													\
ITEM(RBUTTON,             0x02)													\
ITEM(CANCEL,              0x03)													\
ITEM(MBUTTON,             0x04)		/* NOT contiguous with L & RBUTTON */		\
ITEM(XBUTTON1,            0x05)		/* NOT contiguous with L & RBUTTON */		\
ITEM(XBUTTON2,            0x06)		/* NOT contiguous with L & RBUTTON */		\
ITEM(BACK,                0x08)													\
ITEM(TAB,                 0x09)													\
ITEM(CLEAR,               0x0C)													\
ITEM(RETURN,              0x0D)													\
ITEM(SHIFT,               0x10)													\
ITEM(CONTROL,             0x11)													\
ITEM(MENU,                0x12)													\
ITEM(PAUSE,               0x13)													\
ITEM(CAPITAL,             0x14)													\
ITEM(KANA_HANGUL,         0x15)													\
ITEM(JUNJA,               0x17)													\
ITEM(FINAL,               0x18)													\
ITEM(HANJA,               0x19)													\
ITEM(KANJI,               0x19)													\
ITEM(ESCAPE,              0x1B)													\
ITEM(CONVERT,             0x1C)													\
ITEM(NONCONVERT,          0x1D)													\
ITEM(ACCEPT,              0x1E)													\
ITEM(MODECHANGE,          0x1F)													\
ITEM(SPACE,               0x20)													\
ITEM(PRIOR,               0x21)													\
ITEM(NEXT,                0x22)													\
ITEM(END,                 0x23)													\
ITEM(HOME,                0x24)													\
ITEM(LEFT,                0x25)													\
ITEM(UP,                  0x26)													\
ITEM(RIGHT,               0x27)													\
ITEM(DOWN,                0x28)													\
ITEM(SELECT,              0x29)													\
ITEM(PRINT,               0x2A)													\
ITEM(EXECUTE,             0x2B)													\
ITEM(SNAPSHOT,            0x2C)													\
ITEM(INSERT,              0x2D)													\
ITEM(DELETE,              0x2E)													\
ITEM(HELP,                0x2F)													\
/*																				\
 * VKEY_0 - VKEY_9 are the same as ASCII '0' - '9' (0x30 - 0x39)				\
 * VKEY_A - VKEY_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)				\
 */																				\
ITEM(0,                   0x30)													\
ITEM(1,                   0x31)													\
ITEM(2,                   0x32)													\
ITEM(3,                   0x33)													\
ITEM(4,                   0x34)													\
ITEM(5,                   0x35)													\
ITEM(6,                   0x36)													\
ITEM(7,                   0x37)													\
ITEM(8,                   0x38)													\
ITEM(9,                   0x39)													\
ITEM(A,                   0x41)													\
ITEM(B,                   0x42)													\
ITEM(C,                   0x43)													\
ITEM(D,                   0x44)													\
ITEM(E,                   0x45)													\
ITEM(F,                   0x46)													\
ITEM(G,                   0x47)													\
ITEM(H,                   0x48)													\
ITEM(I,                   0x49)													\
ITEM(J,                   0x4A)													\
ITEM(K,                   0x4B)													\
ITEM(L,                   0x4C)													\
ITEM(M,                   0x4D)													\
ITEM(N,                   0x4E)													\
ITEM(O,                   0x4F)													\
ITEM(P,                   0x50)													\
ITEM(Q,                   0x51)													\
ITEM(R,                   0x52)													\
ITEM(S,                   0x53)													\
ITEM(T,                   0x54)													\
ITEM(U,                   0x55)													\
ITEM(V,                   0x56)													\
ITEM(W,                   0x57)													\
ITEM(X,                   0x58)													\
ITEM(Y,                   0x59)													\
ITEM(Z,                   0x5A)													\
ITEM(LWIN,                0x5B)													\
ITEM(RWIN,                0x5C)													\
ITEM(APPS,                0x5D)													\
ITEM(SLEEP,               0x5F)													\
ITEM(NUMPAD0,             0x60)													\
ITEM(NUMPAD1,             0x61)													\
ITEM(NUMPAD2,             0x62)													\
ITEM(NUMPAD3,             0x63)													\
ITEM(NUMPAD4,             0x64)													\
ITEM(NUMPAD5,             0x65)													\
ITEM(NUMPAD6,             0x66)													\
ITEM(NUMPAD7,             0x67)													\
ITEM(NUMPAD8,             0x68)													\
ITEM(NUMPAD9,             0x69)													\
ITEM(MULTIPLY,            0x6A)													\
ITEM(ADD,                 0x6B)													\
ITEM(SEPARATOR,           0x6C)													\
ITEM(SUBTRACT,            0x6D)													\
ITEM(DECIMAL,             0x6E)													\
ITEM(DIVIDE,              0x6F)													\
ITEM(F1,                  0x70)													\
ITEM(F2,                  0x71)													\
ITEM(F3,                  0x72)													\
ITEM(F4,                  0x73)													\
ITEM(F5,                  0x74)													\
ITEM(F6,                  0x75)													\
ITEM(F7,                  0x76)													\
ITEM(F8,                  0x77)													\
ITEM(F9,                  0x78)													\
ITEM(F10,                 0x79)													\
ITEM(F11,                 0x7A)													\
ITEM(F12,                 0x7B)													\
ITEM(F13,                 0x7C)													\
ITEM(F14,                 0x7D)													\
ITEM(F15,                 0x7E)													\
ITEM(F16,                 0x7F)													\
ITEM(F17,                 0x80)													\
ITEM(F18,                 0x81)													\
ITEM(F19,                 0x82)													\
ITEM(F20,                 0x83)													\
ITEM(F21,                 0x84)													\
ITEM(F22,                 0x85)													\
ITEM(F23,                 0x86)													\
ITEM(F24,                 0x87)													\
ITEM(NUMLOCK,             0x90)													\
ITEM(SCROLL,              0x91)													\
/*																				\
 * NEC PC-9800 kbd definitions													\
 */																				\
ITEM(OEM_NEC_EQUAL,       0x92)		/* '=' key on numpad */						\
/*																				\
 * Fujitsu/OASYS kbd definitions												\
 */																				\
ITEM(OEM_FJ_JISHO,        0x92)		/* 'Dictionary' key */						\
ITEM(OEM_FJ_MASSHOU,      0x93)		/* 'Unregister word' key */					\
ITEM(OEM_FJ_TOUROKU,      0x94)		/* 'Register word' key */					\
ITEM(OEM_FJ_LOYA,         0x95)		/* 'Left OYAYUBI' key */					\
ITEM(OEM_FJ_ROYA,         0x96)		/* 'Right OYAYUBI' key */					\
/*																				\
 * VKEY_L* & VKEY_R* - left and right Alt, Ctrl and Shift virtual keys.			\
 * Used only as parameters to GetAsyncKeyState() and GetKeyState().				\
 * No other API or message will distinguish left and right keys in this way.	\
 */																				\
ITEM(LSHIFT,              0xA0)													\
ITEM(RSHIFT,              0xA1)													\
ITEM(LCONTROL,            0xA2)													\
ITEM(RCONTROL,            0xA3)													\
ITEM(LMENU,               0xA4)													\
ITEM(RMENU,               0xA5)													\
ITEM(BROWSER_BACK,        0xA6)													\
ITEM(BROWSER_FORWARD,     0xA7)													\
ITEM(BROWSER_REFRESH,     0xA8)													\
ITEM(BROWSER_STOP,        0xA9)													\
ITEM(BROWSER_SEARCH,      0xAA)													\
ITEM(BROWSER_FAVORITES,   0xAB)													\
ITEM(BROWSER_HOME,        0xAC)													\
ITEM(VOLUME_MUTE,         0xAD)													\
ITEM(VOLUME_DOWN,         0xAE)													\
ITEM(VOLUME_UP,           0xAF)													\
ITEM(MEDIA_NEXT_TRACK,    0xB0)													\
ITEM(MEDIA_PREV_TRACK,    0xB1)													\
ITEM(MEDIA_STOP,          0xB2)													\
ITEM(MEDIA_PLAY_PAUSE,    0xB3)													\
ITEM(LAUNCH_MAIL,         0xB4)													\
ITEM(LAUNCH_MEDIA_SELECT, 0xB5)													\
ITEM(LAUNCH_APP1,         0xB6)													\
ITEM(LAUNCH_APP2,         0xB7)													\
ITEM(OEM_1,               0xBA)		/* ';:' for US */							\
ITEM(OEM_PLUS,            0xBB)		/* '+' any country */						\
ITEM(OEM_COMMA,           0xBC)		/* ',' any country */						\
ITEM(OEM_MINUS,           0xBD)		/* '-' any country */						\
ITEM(OEM_PERIOD,          0xBE)		/* '.' any country */						\
ITEM(OEM_2,               0xBF)		/* '/?' for US */							\
ITEM(OEM_3,               0xC0)		/* '`~' for US */							\
ITEM(OEM_4,               0xDB)		/* '[{' for US */							\
ITEM(OEM_5,               0xDC)		/* '\|' for US */							\
ITEM(OEM_6,               0xDD)		/* ']}' for US */							\
ITEM(OEM_7,               0xDE)		/* ''"' for US */							\
ITEM(OEM_8,               0xDF)													\
/*																				\
 * Various extended or enhanced keyboards										\
 */																				\
ITEM(OEM_AX,              0xE1)		/* 'AX' key on Japanese AX kbd */			\
ITEM(OEM_102,             0xE2)		/* "<>" or "\|" on RT 102-key kbd */		\
ITEM(ICO_HELP,            0xE3)		/* Help key on ICO */						\
ITEM(ICO_00,              0xE4)		/* 00 key on ICO */							\
ITEM(PROCESSKEY,          0xE5)													\
ITEM(ICO_CLEAR,           0xE6)													\
ITEM(PACKET,              0xE7)													\
/*																				\
 * Nokia/Ericsson definitions													\
 */																				\
ITEM(OEM_RESET,           0xE9)													\
ITEM(OEM_JUMP,            0xEA)													\
ITEM(OEM_PA1,             0xEB)													\
ITEM(OEM_PA2,             0xEC)													\
ITEM(OEM_PA3,             0xED)													\
ITEM(OEM_WSCTRL,          0xEE)													\
ITEM(OEM_CUSEL,           0xEF)													\
ITEM(OEM_ATTN,            0xF0)													\
ITEM(OEM_FINISH,          0xF1)													\
ITEM(OEM_COPY,            0xF2)													\
ITEM(OEM_AUTO,            0xF3)													\
ITEM(OEM_ENLW,            0xF4)													\
ITEM(OEM_BACKTAB,         0xF5)													\
ITEM(ATTN,                0xF6)													\
ITEM(CRSEL,               0xF7)													\
ITEM(EXSEL,               0xF8)													\
ITEM(EREOF,               0xF9)													\
ITEM(PLAY,                0xFA)													\
ITEM(ZOOM,                0xFB)													\
ITEM(NONAME,              0xFC)													\
ITEM(PA1,                 0xFD)													\
ITEM(OEM_CLEAR,           0xFE)													\
/*																				\
 * Friendly names for common keys (US)											\
 */																				\
ITEM(SEMICOLON,           0xBA)		/* OEM_1 */									\
ITEM(SLASH,               0xBF)		/* OEM_2 */									\
ITEM(GRAVE_ACCENT,        0xC0)		/* OEM_3 */									\
ITEM(RIGHT_BRACKET,       0xDB)		/* OEM_4 */									\
ITEM(BACKSLASH,           0xDC)		/* OEM_5 */									\
ITEM(LEFT_BRACKET,        0xDD)		/* OEM_6 */									\
ITEM(APOSTROPHE,          0xDE)		/* OEM_7 */									\

#define GUI_DEFINE_KEYBOARD_CODE_ENUM_ITEM(NAME, CODE) _##NAME = CODE,
		enum class VKEY
		{
			_UNKNOWN = -1,
			GUI_DEFINE_KEYBOARD_CODE(GUI_DEFINE_KEYBOARD_CODE_ENUM_ITEM)
		};
#undef GUI_DEFINE_KEYBOARD_CODE_ENUM_ITEM
		static bool operator == (VKEY a, VKEY b) { return (vint)a == (vint)b; }
		static bool operator != (VKEY a, VKEY b) { return (vint)a != (vint)b; }
		static bool operator <  (VKEY a, VKEY b) { return (vint)a <  (vint)b; }
		static bool operator <= (VKEY a, VKEY b) { return (vint)a <= (vint)b; }
		static bool operator >  (VKEY a, VKEY b) { return (vint)a >  (vint)b; }
		static bool operator >= (VKEY a, VKEY b) { return (vint)a >= (vint)b; }
		static VKEY operator &  (VKEY a, VKEY b) { return (VKEY)((vint)a & (vint)b); }
		static VKEY operator |  (VKEY a, VKEY b) { return (VKEY)((vint)a | (vint)b); }

	}
}

#endif