using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Parser;

namespace ParserTest
{
    [TestClass]
    public class TypeDeclTest
    {
        static string Convert(string input)
        {
            var tokens = input.Split(" ".ToCharArray(), StringSplitOptions.RemoveEmptyEntries);
            int index = 0;
            TypeDecl decl = null;
            string name = null;
            Assert.IsTrue(TypeDecl.ParseType(tokens, ref index, out decl, out name));

            if (name == null)
            {
                return decl.ToString();
            }
            else
            {
                return name + " : " + decl.ToString();
            }
        }

        [TestMethod]
        public void TestSimpleType()
        {
            Assert.AreEqual(
                "int",
                Convert("int")
                );
            Assert.AreEqual(
                "a : int",
                Convert("int a")
                );
        }

        [TestMethod]
        public void TestDecoratedType()
        {
            Assert.AreEqual(
                "const int",
                Convert("const int")
                );
            Assert.AreEqual(
                "a : const int",
                Convert("const int a")
                );
            Assert.AreEqual(
                "const int",
                Convert("int const")
                );
            Assert.AreEqual(
                "a : const int",
                Convert("int const a")
                );

            Assert.AreEqual(
                "volatile int",
                Convert("volatile int")
                );
            Assert.AreEqual(
                "a : volatile int",
                Convert("volatile int a")
                );
            Assert.AreEqual(
                "volatile int",
                Convert("int volatile")
                );
            Assert.AreEqual(
                "a : volatile int",
                Convert("int volatile a")
                );

            Assert.AreEqual(
                "signed int",
                Convert("signed int")
                );
            Assert.AreEqual(
                "a : signed int",
                Convert("signed int a")
                );

            Assert.AreEqual(
                "unsigned int",
                Convert("unsigned int")
                );
            Assert.AreEqual(
                "a : unsigned int",
                Convert("unsigned int a")
                );

            Assert.AreEqual(
                "* int",
                Convert("int *")
                );
            Assert.AreEqual(
                "a : * int",
                Convert("int * a")
                );

            Assert.AreEqual(
                "& int",
                Convert("int &")
                );
            Assert.AreEqual(
                "a : & int",
                Convert("int & a")
                );

            Assert.AreEqual(
                "&& int",
                Convert("int &&")
                );
            Assert.AreEqual(
                "a : && int",
                Convert("int && a")
                );
        }

        [TestMethod]
        public void TestDecoratedTypeProperity()
        {
            Assert.AreEqual(
                "* const int",
                Convert("const int *")
                );
            Assert.AreEqual(
                "a : * const int",
                Convert("const int * a")
                );

            Assert.AreEqual(
                "const * int",
                Convert("int * const")
                );
            Assert.AreEqual(
                "a : const * int",
                Convert("int * const a")
                );

            Assert.AreEqual(
                "const * const int",
                Convert("const int * const")
                );
            Assert.AreEqual(
                "a : const * const int",
                Convert("const int * const a")
                );
        }

        [TestMethod]
        public void TestGenericType()
        {
            Assert.AreEqual(
                "vector<int>",
                Convert("vector < int >")
                );
            Assert.AreEqual(
                "a : vector<int>",
                Convert("vector < int > a")
                );

            Assert.AreEqual(
                "vector<int, float>",
                Convert("vector < int , float >")
                );
            Assert.AreEqual(
                "a : vector<int, float>",
                Convert("vector < int , float > a")
                );
        }

        [TestMethod]
        public void TestSubType()
        {
            Assert.AreEqual(
                "Outer::Inner",
                Convert("Outer : : Inner")
                );
            Assert.AreEqual(
                "a : Outer::Inner",
                Convert("Outer : : Inner a")
                );

            Assert.AreEqual(
                "Outer::Inner::MostInner",
                Convert("Outer : : Inner : : MostInner")
                );
            Assert.AreEqual(
                "a : Outer::Inner::MostInner",
                Convert("Outer : : Inner : : MostInner a")
                );
        }

        [TestMethod]
        public void TestClassMemberType()
        {
            Assert.AreEqual(
                "Class::(int)",
                Convert("int Class : :")
                );
            Assert.AreEqual(
                "a : Class::(int)",
                Convert("int Class : : a")
                );
        }

        [TestMethod]
        public void TestArrayType()
        {
            Assert.AreEqual(
                "array int",
                Convert("int [ ]")
                );
            Assert.AreEqual(
                "a : array int",
                Convert("int a [ ]")
                );
            Assert.AreEqual(
                "a : array int",
                Convert("int ( a ) [ ]")
                );
            Assert.AreEqual(
                "a : & array int",
                Convert("int ( & a ) [ ]")
                );

            Assert.AreEqual(
                "array int",
                Convert("int [ 10 ]")
                );
            Assert.AreEqual(
                "a : array int",
                Convert("int a [ 10 ]")
                );
            Assert.AreEqual(
                "a : array int",
                Convert("int ( a ) [ 10 ]")
                );
            Assert.AreEqual(
                "a : & array int",
                Convert("int ( & a ) [ 10 ]")
                );

            Assert.AreEqual(
                "array array int",
                Convert("int [ 10 ] [ 20 ]")
                );
            Assert.AreEqual(
                "a : array array int",
                Convert("int a [ 10 ] [ 20 ]")
                );
            Assert.AreEqual(
                "a : array array int",
                Convert("int ( a ) [ 10 ] [ 20 ]")
                );
            Assert.AreEqual(
                "a : & array array int",
                Convert("int ( & a ) [ 10 ] [ 20 ]")
                );

            Assert.AreEqual(
                "array array int",
                Convert("int [ sizeof ( double ) ] [ call ( 1 , 2 , 3 ) ]")
                );
            Assert.AreEqual(
                "a : array array int",
                Convert("int a [ sizeof ( double ) ] [ call ( 1 , 2 , 3 ) ]")
                );
            Assert.AreEqual(
                "a : array array int",
                Convert("int ( a ) [ sizeof ( double ) ] [ call ( 1 , 2 , 3 ) ]")
                );
            Assert.AreEqual(
                "a : & array array int",
                Convert("int ( & a ) [ sizeof ( double ) ] [ call ( 1 , 2 , 3 ) ]")
                );
        }

        [TestMethod]
        public void TestFunctionType()
        {
            Assert.AreEqual(
                "function (a : int, float, double) : int",
                Convert("int ( int a , float , double )")
                );
            Assert.AreEqual(
                "a : function (a : int, float, double) : int",
                Convert("int a ( int a , float , double )")
                );
            Assert.AreEqual(
                "a : function (a : int, float, double) : int",
                Convert("int ( a ) ( int a , float , double )")
                );
            Assert.AreEqual(
                "a : * function (a : int, float, double) : int",
                Convert("int ( * a ) ( int a , float , double )")
                );

            Assert.AreEqual(
                "function __stdcall (a : int, float, double) : int",
                Convert("int __stdcall ( int a , float , double )")
                );
            Assert.AreEqual(
                "a : function __stdcall (a : int, float, double) : int",
                Convert("int __stdcall a ( int a , float , double )")
                );
            Assert.AreEqual(
                "a : function __stdcall (a : int, float, double) : int",
                Convert("int __stdcall ( a ) ( int a , float , double )")
                );
            Assert.AreEqual(
                "a : * function __stdcall (a : int, float, double) : int",
                Convert("int __stdcall ( * a ) ( int a , float , double )")
                );

            Assert.AreEqual(
                "* Class::(function __thiscall const (a : int, float, double) : int)",
                Convert("int ( __thiscall Class : : * ) ( int a , float , double ) const")
                );
            Assert.AreEqual(
                "callback : * Class::(function __thiscall (a : int, float, double) : int)",
                Convert("int ( __thiscall Class : : * callback ) ( int a , float , double )")
                );
        }

        [TestMethod]
        public void TestBossType()
        {
            Assert.AreEqual(
                "var : && array array * Class::(function __thiscall const (a : * array int, b : * function () : void) : & array int)",
                Convert("int ( & ( __thiscall Class : : * ( && var ) [ 10 ] [ 20 ] ) ( int ( * a ) [ 30 ] , void ( * b ) ( ) ) const ) [ 40 ]")
                );
        }
    }
}
