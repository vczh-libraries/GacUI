/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Common Types

Classes:
***********************************************************************/

#ifndef VCZH_PRESENTATION_GUITYPES
#define VCZH_PRESENTATION_GUITYPES

#include <VlppOS.h>
#include <VlppGlrParser.h>
#include <VlppWorkflowLibrary.h>

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

#define GUI_DEFINE_COMPARE_OPERATORS(TYPE)\
		auto operator<=>(const TYPE&) const = default;\

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

			GUI_DEFINE_COMPARE_OPERATORS(TextPos)
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

			GUI_DEFINE_COMPARE_OPERATORS(GridPos)
		};

/***********************************************************************
Coordinate
***********************************************************************/



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

			GUI_DEFINE_COMPARE_OPERATORS(Point_<T>)
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

			GUI_DEFINE_COMPARE_OPERATORS(Size_<T>)
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

			GUI_DEFINE_COMPARE_OPERATORS(Rect_<T>)

			Point_<T> LeftTop() const
			{
				return Point_<T>(x1, y1);
			}

			Point_<T> RightBottom() const
			{
				return Point_<T>(x2, y2);
			}

			Size_<T> GetSize() const
			{
				return Size_<T>(x2 - x1, y2 - y1);
			}

			T Left() const
			{
				return x1;
			}

			T Right() const
			{
				return x2;
			}

			T Width() const
			{
				return x2 - x1;
			}

			T Top() const
			{
				return y1;
			}

			T Bottom() const
			{
				return y2;
			}

			T Height() const
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

			bool Contains(Point_<T> p) const
			{
				return x1 <= p.x && p.x < x2 && y1 <= p.y && p.y < y2;
			}

			bool Contains(Rect_<T> r) const
			{
				return x1 <= r.x1 && r.x2 <= x2 && y1 <= r.y1 && r.y2 <= y2;
			}

			Rect_<T> Intersect(Rect_<T> r)  const
			{
				Rect_<T> result = r;
				if (r.x1 < x1) r.x1 = x1;
				if (r.x2 > x2) r.x2 = x2;
				if (r.y1 < y1) r.y1 = y1;
				if (r.y2 > y2) r.y2 = y2;
				return r;
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

			std::strong_ordering operator<=>(const Color& c) const { return value <=> c.value; }
			bool operator==(const Color& c) const { return value == c.value; }

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

			GUI_DEFINE_COMPARE_OPERATORS(Margin_<T>)
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
			
			GUI_DEFINE_COMPARE_OPERATORS(FontProperties)
		};

/***********************************************************************
Keys
***********************************************************************/

#define GUI_DEFINE_KEYBOARD_WINDOWS_NAME(ITEM)									\
ITEM(BACK,					L"Backspace")\
ITEM(TAB,					L"Tab")\
ITEM(RETURN,				L"Enter")\
ITEM(SHIFT,					L"Shift")\
ITEM(CONTROL,				L"Ctrl")\
ITEM(MENU,					L"Alt")\
ITEM(CAPITAL,				L"Caps Lock")\
ITEM(ESCAPE,				L"Esc")\
ITEM(SPACE,					L"Space")\
ITEM(PRIOR,					L"Page Up")\
ITEM(NEXT,					L"Page Down")\
ITEM(END,					L"End")\
ITEM(HOME,					L"Home")\
ITEM(LEFT,					L"Left")\
ITEM(UP,					L"Up")\
ITEM(RIGHT,					L"Right")\
ITEM(DOWN,					L"Down")\
ITEM(SNAPSHOT,				L"Sys Req")\
ITEM(INSERT,				L"Insert")\
ITEM(DELETE,				L"Delete")\
ITEM(0,						L"0")\
ITEM(1,						L"1")\
ITEM(2,						L"2")\
ITEM(3,						L"3")\
ITEM(4,						L"4")\
ITEM(5,						L"5")\
ITEM(6,						L"6")\
ITEM(7,						L"7")\
ITEM(8,						L"8")\
ITEM(9,						L"9")\
ITEM(A,						L"A")\
ITEM(B,						L"B")\
ITEM(C,						L"C")\
ITEM(D,						L"D")\
ITEM(E,						L"E")\
ITEM(F,						L"F")\
ITEM(G,						L"G")\
ITEM(H,						L"H")\
ITEM(I,						L"I")\
ITEM(J,						L"J")\
ITEM(K,						L"K")\
ITEM(L,						L"L")\
ITEM(M,						L"M")\
ITEM(N,						L"N")\
ITEM(O,						L"O")\
ITEM(P,						L"P")\
ITEM(Q,						L"Q")\
ITEM(R,						L"R")\
ITEM(S,						L"S")\
ITEM(T,						L"T")\
ITEM(U,						L"U")\
ITEM(V,						L"V")\
ITEM(W,						L"W")\
ITEM(X,						L"X")\
ITEM(Y,						L"Y")\
ITEM(Z,						L"Z")\
ITEM(NUMPAD0,				L"Num 0")\
ITEM(NUMPAD1,				L"Num 1")\
ITEM(NUMPAD2,				L"Num 2")\
ITEM(NUMPAD3,				L"Num 3")\
ITEM(NUMPAD4,				L"Num 4")\
ITEM(NUMPAD5,				L"Num 5")\
ITEM(NUMPAD6,				L"Num 6")\
ITEM(NUMPAD7,				L"Num 7")\
ITEM(NUMPAD8,				L"Num 8")\
ITEM(NUMPAD9,				L"Num 9")\
ITEM(MULTIPLY,				L"Num *")\
ITEM(ADD,					L"Num +")\
ITEM(SUBTRACT,				L"Num -")\
ITEM(DECIMAL,				L"Num Del")\
ITEM(DIVIDE,				L"/")\
ITEM(F1,					L"F1")\
ITEM(F2,					L"F2")\
ITEM(F3,					L"F3")\
ITEM(F4,					L"F4")\
ITEM(F5,					L"F5")\
ITEM(F6,					L"F6")\
ITEM(F7,					L"F7")\
ITEM(F8,					L"F8")\
ITEM(F9,					L"F9")\
ITEM(F10,					L"F10")\
ITEM(F11,					L"F11")\
ITEM(F12,					L"F12")\
ITEM(NUMLOCK,				L"Pause")\
ITEM(SCROLL,				L"Scroll Lock")\
ITEM(BROWSER_HOME,			L"BROWSER_HOME")\
ITEM(VOLUME_MUTE,			L"VOLUME_MUTE")\
ITEM(VOLUME_DOWN,			L"VOLUME_DOWN")\
ITEM(VOLUME_UP,				L"VOLUME_UP")\
ITEM(MEDIA_NEXT_TRACK,		L"MEDIA_NEXT_TRACK")\
ITEM(MEDIA_PREV_TRACK,		L"MEDIA_PREV_TRACK")\
ITEM(MEDIA_STOP,			L"MEDIA_STOP")\
ITEM(MEDIA_PLAY_PAUSE,		L"MEDIA_PLAY_PAUSE")\
ITEM(LAUNCH_APP2,			L"LAUNCH_APP2")\
ITEM(OEM_PLUS,				L"=")\
ITEM(OEM_COMMA,				L",")\
ITEM(OEM_MINUS,				L"-")\
ITEM(OEM_PERIOD,			L".")\
ITEM(OEM_102,				L"\\")\
ITEM(SEMICOLON,				L";")\
ITEM(SLASH,					L"/")\
ITEM(GRAVE_ACCENT,			L"`")\
ITEM(LEFT_BRACKET,			L"[")\
ITEM(BACKSLASH,				L"\\")\
ITEM(RIGHT_BRACKET,			L"]")\
ITEM(APOSTROPHE,			L"'")\



	}
}

#endif