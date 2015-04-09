using System;
using System.Text;
using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Parser;

namespace ParserTest
{
    [TestClass]
    public class SymbolDeclTest
    {
        static SymbolDecl Convert(string input)
        {
            var tokens = input.Split(" \r\n".ToCharArray(), StringSplitOptions.RemoveEmptyEntries);
            int index = 0;

            var decl = new SymbolDecl();
            SymbolDecl.ParseSymbols(tokens, ref index, decl);
            Assert.AreEqual(tokens.Length, index);

            return decl;
        }

        [TestMethod]
        public void TestNamespace()
        {
            var global = Convert(@"
                namespace vl { }
                ");
            Assert.AreEqual(1, global.Children.Count);
            Assert.AreEqual("namespace vl", global.Children[0].ToString());
        }

        [TestMethod]
        public void TestUsingNamespace()
        {
            var global = Convert(@"
                using namespace vl ;
                using namespace vl : : collections ;
                using namespace vl : : parsing : : xml ;
                ");
            Assert.AreEqual(3, global.Children.Count);
            Assert.AreEqual("using namespace vl", global.Children[0].ToString());
            Assert.AreEqual("using namespace vl::collections", global.Children[1].ToString());
            Assert.AreEqual("using namespace vl::parsing::xml", global.Children[2].ToString());
        }

        [TestMethod]
        public void TestClass()
        {
            var global = Convert(@"
                class A ;
                struct A ;
                union A ;
                class A { } ;
                struct A { } ;
                union A { } ;
                class A : public B , protected C < D > , private typename E < F > : : template G < H > , I { } ;
                struct A : public B , protected C < D > , private typename E < F > : : template G < H > , I { } a ;
                ");
            Assert.AreEqual(6, global.Children.Count);
            Assert.AreEqual("class A", global.Children[0].ToString());
            Assert.AreEqual("struct A", global.Children[1].ToString());
            Assert.AreEqual("union A", global.Children[2].ToString());
            Assert.AreEqual("class A : public B, protected C<D>, private E<F>::G<H>, private I", global.Children[3].ToString());
            Assert.AreEqual("struct A : public B, protected C<D>, private E<F>::G<H>, public I", global.Children[4].ToString());
            Assert.AreEqual("var a : A", global.Children[5].ToString());
        }

        [TestMethod]
        public void TestFriendClass()
        {
            var global = Convert(@"
                friend class Class ;
                friend class Outer : : Inner ;
                friend class typename List < T > : : Inner ;
                friend class typename List < T > : : template Inner < T > ;
                template < typename T > friend class Ptr ;
                ");
            Assert.AreEqual(0, global.Children.Count);
        }

        [TestMethod]
        public void TestFriendFunction()
        {
            var global = Convert(@"
                friend void Func ( void ) ;
                friend int Class : : Func ( int ) ;
                template < typename T > friend T Class : : Func ( T ) ;
                ");
            Assert.AreEqual(0, global.Children.Count);
        }

        [TestMethod]
        public void TestUsingFunction()
        {
            var global = Convert(@"
                using Class : : Func ;
                using Class < T > : : operator [ ] ;
                ");
            Assert.AreEqual(0, global.Children.Count);
        }

        [TestMethod]
        public void TestClassField()
        {
            var global = Convert(@"
                int a ;
                int a = 0 ;
                int ( * a ) ( int ) ;
                int ( * a ) ( void ) ;
                ");
            Assert.AreEqual(4, global.Children.Count);
            Assert.AreEqual("var a : int", global.Children[0].ToString());
            Assert.AreEqual("var a : int", global.Children[1].ToString());
            Assert.AreEqual("var a : * function (int) : int", global.Children[2].ToString());
            Assert.AreEqual("var a : * function () : int", global.Children[3].ToString());
        }

        [TestMethod]
        public void TestClassFunction()
        {
            var global = Convert(@"
                int a ( int ) override ;
                virtual int a ( void ) const ;
                virtual int a ( void ) const = 0 ;
                virtual int a ( int ( * x ) ( ) = 0 ) const = 0 ;
                bool operator < < ( int ) ;
                operator bool ( int ) ;
                ");
            Assert.AreEqual(6, global.Children.Count);
            Assert.AreEqual("function a : function (int) : int", global.Children[0].ToString());
            Assert.AreEqual("virtual function a : function const () : int", global.Children[1].ToString());
            Assert.AreEqual("abstract function a : function const () : int", global.Children[2].ToString());
            Assert.AreEqual("abstract function a : function const (x : * function () : int) : int", global.Children[3].ToString());
            Assert.AreEqual("function operator << : function (int) : bool", global.Children[4].ToString());
            Assert.AreEqual("function operator : function (int) : bool", global.Children[5].ToString());
        }

        [TestMethod]
        public void TestClassCtorDtor()
        {
            var global = Convert(@"
                A ( ) ;
                ~ A ( ) ;
                ");
            Assert.AreEqual(2, global.Children.Count);
            Assert.AreEqual("ctor A : function () : void", global.Children[0].ToString());
            Assert.AreEqual("dtor ~A : function () : void", global.Children[1].ToString());
        }

        [TestMethod]
        public void TestClassGroupedField()
        {
            var global = Convert(@"
                union
                {
                    int a ;
                    struct
                    {
                        int b ;
                        int c ;
                    } ;
                } ;
                ");
            Assert.AreEqual(1, global.Children.Count);
            Assert.AreEqual("union", global.Children[0].ToString());

            Assert.AreEqual(2, global.Children[0].Children.Count);
            Assert.AreEqual("var a : int", global.Children[0].Children[0].ToString());
            Assert.AreEqual("struct", global.Children[0].Children[1].ToString());

            Assert.AreEqual(2, global.Children[0].Children[1].Children.Count);
            Assert.AreEqual("var b : int", global.Children[0].Children[1].Children[0].ToString());
            Assert.AreEqual("var c : int", global.Children[0].Children[1].Children[1].ToString());
        }

        [TestMethod]
        public void TestEnum()
        {
            var global = Convert(@"
                enum A ;
                enum class A : int ;
                enum A
                {
                    X , Y , Z ,
                } ;
                enum class A : int
                {
                    X = 1 , Y = 2 , Z = 3
                } a ;
                ");
            Assert.AreEqual(3, global.Children.Count);

            Assert.AreEqual("enum A", global.Children[0].ToString());
            Assert.AreEqual(3, global.Children[0].Children.Count);
            Assert.AreEqual("enum_item X", global.Children[0].Children[0].ToString());
            Assert.AreEqual("enum_item Y", global.Children[0].Children[1].ToString());
            Assert.AreEqual("enum_item Z", global.Children[0].Children[2].ToString());

            Assert.AreEqual("enum A", global.Children[1].ToString());
            Assert.AreEqual(3, global.Children[1].Children.Count);
            Assert.AreEqual("enum_item X", global.Children[1].Children[0].ToString());
            Assert.AreEqual("enum_item Y", global.Children[1].Children[1].ToString());
            Assert.AreEqual("enum_item Z", global.Children[1].Children[2].ToString());

            Assert.AreEqual("var a : A", global.Children[2].ToString());
        }

        [TestMethod]
        public void TestTypedef()
        {
            var global = Convert(@"
                typedef int a ;
                typedef int ( * a ) ( int ) ;
                typedef int ( * a ) ( void ) ;
                using a = int ;
                using a = int ( * ) ( int ) ;
                using a = int ( * ) ( void ) ;
                ");
            Assert.AreEqual(6, global.Children.Count);
            Assert.AreEqual("typedef a : int", global.Children[0].ToString());
            Assert.AreEqual("typedef a : * function (int) : int", global.Children[1].ToString());
            Assert.AreEqual("typedef a : * function () : int", global.Children[2].ToString());
            Assert.AreEqual("typedef a : int", global.Children[3].ToString());
            Assert.AreEqual("typedef a : * function (int) : int", global.Children[4].ToString());
            Assert.AreEqual("typedef a : * function () : int", global.Children[5].ToString());
        }

        [TestMethod]
        public void TestTemplateFunction()
        {
            var global = Convert(@"
                template < typename T > T a ( T ) override ;
                template < typename T > virtual T a ( void ) const ;
                template < typename T > virtual T a ( void ) const = 0 ;
                ");
            Assert.AreEqual(3, global.Children.Count);
            Assert.AreEqual("template<T> function a : function (T) : T", global.Children[0].ToString());
            Assert.AreEqual("template<T> virtual function a : function const () : T", global.Children[1].ToString());
            Assert.AreEqual("template<T> abstract function a : function const () : T", global.Children[2].ToString());
        }

        [TestMethod]
        public void TestTemplateClass()
        {
            var global = Convert(@"
                template < typename T > struct IsPOD { } ;
                template < typename T > struct IsPOD < int > { } ;
                template < typename T = U > struct IsPOD < shared_ptr < T > > { } ;
                ");
            Assert.AreEqual(3, global.Children.Count);
            Assert.AreEqual("template<T> struct IsPOD", global.Children[0].ToString());
            Assert.AreEqual("template<T> specialization<int> struct IsPOD", global.Children[1].ToString());
            Assert.AreEqual("template<T> specialization<shared_ptr<T>> struct IsPOD", global.Children[2].ToString());
        }

        [TestMethod]
        public void TestTemplateTypedef()
        {
            var global = Convert(@"
                template < typename T > using X = vector < T > ;
                template < typename T > using X < int > = list < int > ;
                template < typename T = U > using X < shared_ptr < T > > = vector < unique_ptr < T > > ;
                ");
            Assert.AreEqual(3, global.Children.Count);
            Assert.AreEqual("template<T> typedef X : vector<T>", global.Children[0].ToString());
            Assert.AreEqual("template<T> specialization<int> typedef X : list<int>", global.Children[1].ToString());
            Assert.AreEqual("template<T> specialization<shared_ptr<T>> typedef X : vector<unique_ptr<T>>", global.Children[2].ToString());
        }

        [TestMethod]
        public void TestComment()
        {
            var global = Convert(@"
                ///<summary>
                ///This_is_a_comment
                ///</summary>
                class A
                {
                public :
                    ///<summary>
                    ///This_is_also_a_comment
                    ///</summary>
                    void B ( ) ;
                } ;
                ");
            Assert.AreEqual(1, global.Children.Count);
            Assert.AreEqual("class A", global.Children[0].ToString());

            Assert.AreEqual(1, global.Children[0].Children.Count);
            Assert.AreEqual("function B : function () : void", global.Children[0].Children[0].ToString());
        }
    }
}
