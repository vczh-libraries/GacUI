using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

// These tests use native IUIAutomation vtables and balance every returned COM
// pointer. No AutomationElement RCWs or forced garbage collection own lifetimes.
public static class GacUILifetimeTests
{
    [ComVisible(true), Guid("40cd37d4-c756-4b0c-8c6f-bddfeeb13b50"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IProperties { void HandlePropertyChangedEvent(IntPtr sender, int id, [MarshalAs(UnmanagedType.Struct)] object value); }
    [ComVisible(true), Guid("c270f6b5-5c69-4290-9745-7a7f97169468"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IFocus { void HandleFocusChangedEvent(IntPtr sender); }
    [DllImport("user32.dll")] static extern IntPtr GetForegroundWindow();
    [DllImport("ole32.dll")] static extern int CoCreateInstance(ref Guid clsid, IntPtr outer, uint context, ref Guid iid, out IntPtr value);
    delegate bool WindowCallback(IntPtr hwnd, IntPtr parameter);
    [DllImport("user32.dll")] static extern bool EnumWindows(WindowCallback callback, IntPtr parameter);
    [DllImport("user32.dll")] static extern uint GetWindowThreadProcessId(IntPtr hwnd, out uint process);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] static extern int GetWindowText(IntPtr hwnd, StringBuilder text, int count);
    [UnmanagedFunctionPointer(CallingConvention.StdCall)] delegate int GetPointer(IntPtr self, out IntPtr result);
    [UnmanagedFunctionPointer(CallingConvention.StdCall)] delegate int Element(IntPtr self, IntPtr input, out IntPtr result);
    [UnmanagedFunctionPointer(CallingConvention.StdCall)] delegate int Property(IntPtr self, int id, [MarshalAs(UnmanagedType.Struct)] out object value);
    [UnmanagedFunctionPointer(CallingConvention.StdCall)] delegate int GetPattern(IntPtr self, int id, ref Guid iid, out IntPtr result);
    [UnmanagedFunctionPointer(CallingConvention.StdCall)] delegate int Action(IntPtr self);
    [UnmanagedFunctionPointer(CallingConvention.StdCall)] delegate int Text(IntPtr self, int count, [MarshalAs(UnmanagedType.BStr)] out string result);
    [UnmanagedFunctionPointer(CallingConvention.StdCall)] delegate int RuntimeId(IntPtr self, [MarshalAs(UnmanagedType.SafeArray, SafeArraySubType=VarEnum.VT_I4)] out int[] result);
    [UnmanagedFunctionPointer(CallingConvention.StdCall)] delegate int AddProperties(IntPtr self, IntPtr element, int scope, IntPtr cache, [MarshalAs(UnmanagedType.Interface)] IProperties handler, [In, MarshalAs(UnmanagedType.LPArray, SizeParamIndex=6)] int[] ids, int count);
    [UnmanagedFunctionPointer(CallingConvention.StdCall)] delegate int RemoveProperties(IntPtr self, IntPtr element, [MarshalAs(UnmanagedType.Interface)] IProperties handler);
    [UnmanagedFunctionPointer(CallingConvention.StdCall)] delegate int AddFocus(IntPtr self, IntPtr cache, [MarshalAs(UnmanagedType.Interface)] IFocus handler);
    [UnmanagedFunctionPointer(CallingConvention.StdCall)] delegate int RemoveFocus(IntPtr self, [MarshalAs(UnmanagedType.Interface)] IFocus handler);
    static T Method<T>(IntPtr self, int slot) where T : class { return (T)(object)Marshal.GetDelegateForFunctionPointer(Marshal.ReadIntPtr(Marshal.ReadIntPtr(self), slot * IntPtr.Size), typeof(T)); }
    static void Check(int hr) { Marshal.ThrowExceptionForHR(hr); }
    static int assertions;
    static void Require(bool value, string message) { if (!value) throw new Exception(message); assertions++; }
    static void Wait(Func<bool> condition, string message) { var watch=Stopwatch.StartNew(); while (!condition()) { if(watch.ElapsedMilliseconds>10000) throw new Exception("Timeout: "+message); Thread.Sleep(20); } assertions++; }
    sealed class Reference : IDisposable
    {
        public IntPtr Pointer;
        public Reference(IntPtr value) { Pointer=value; }
        public void Dispose() { if(Pointer!=IntPtr.Zero) { Marshal.Release(Pointer); Pointer=IntPtr.Zero; } }
    }
    static Reference Pointer(IntPtr self, int slot) { IntPtr value; Check(Method<GetPointer>(self,slot)(self,out value)); return new Reference(value); }
    static Reference Pattern(Reference element, int id, string guid) { IntPtr value; var iid=new Guid(guid); Check(Method<GetPattern>(element.Pointer,14)(element.Pointer,id,ref iid,out value)); Require(value!=IntPtr.Zero,"Requested pattern "+id); return new Reference(value); }
    static string Name(IntPtr pointer) { object value; Check(Method<Property>(pointer,10)(pointer,30005,out value)); return (string)value; }
    static string Identity(Reference element) { int[] value; Check(Method<RuntimeId>(element.Pointer,4)(element.Pointer,out value)); return string.Join(".",value); }
    static string Contents(Reference range) { string text; Check(Method<Text>(range.Pointer,12)(range.Pointer,-1,out text)); return text; }
    static Reference Range(Reference element) { using(var pattern=Pattern(element,10014,"32eba289-3583-42c9-9c59-3b6d9a1e9b6a")) return Pointer(pattern.Pointer,7); }
    static void Unavailable(Reference element, Reference range)
    {
        Wait(()=> { object value; return Method<Property>(element.Pointer,10)(element.Pointer,30005,out value)==unchecked((int)0x80040201); },"retired element unavailable");
        string text;
        Require(Method<Text>(range.Pointer,12)(range.Pointer,-1,out text)==unchecked((int)0x80040201),"retired range unavailable");
    }
    static IntPtr Window(int process, string title)
    {
        IntPtr result=IntPtr.Zero;
        EnumWindows((h,p)=> { uint id; GetWindowThreadProcessId(h,out id); if(id==process) {var name=new StringBuilder(256); GetWindowText(h,name,name.Capacity); if(name.ToString()==title) result=h;} return true;},IntPtr.Zero);
        return result;
    }
    [ComVisible(true), ClassInterface(ClassInterfaceType.None)]
    public sealed class Client : IProperties, IFocus, IDisposable
    {
        IntPtr instance, walker;
        Reference root, subscribed;
        public int Changes, FocusChanges, DescendantChanges;
        public Client(IntPtr hwnd)
        {
            var clsid=new Guid("ff48dba4-60ef-4201-aa87-54103eef594e"); var iid=new Guid("30cbe57d-d9d0-452a-ab13-7ac5ac4825ee");
            Check(CoCreateInstance(ref clsid,IntPtr.Zero,1,ref iid,out instance));
            IntPtr value; Check(Method<Element>(instance,6)(instance,hwnd,out value)); root=new Reference(value);
            Check(Method<GetPointer>(instance,16)(instance,out walker));
        }
        Reference Search(IntPtr parent, string name)
        {
            if(Name(parent)==name) { Marshal.AddRef(parent); return new Reference(parent); }
            IntPtr child; Check(Method<Element>(walker,4)(walker,parent,out child));
            while(child!=IntPtr.Zero)
            {
                using(var current=new Reference(child))
                {
                    var found=Search(child,name); if(found!=null) return found;
                    Check(Method<Element>(walker,6)(walker,child,out child));
                }
            }
            return null;
        }
        Reference Find(string name) { Reference result=null; Wait(()=> { result=Search(root.Pointer,name); return result!=null; },"find "+name); return result; }
        public void Invoke(string name) { using(var element=Find(name)) using(var pattern=Pattern(element,10000,"fb377fbe-8ea6-46d5-9c73-6499642d3059")) Check(Method<Action>(pattern.Pointer,3)(pattern.Pointer)); Thread.Sleep(70); }
        public void Listen(string window)
        {
            subscribed=Find(window);
            Check(Method<AddProperties>(instance,34)(instance,subscribed.Pointer,7,IntPtr.Zero,this,new int[]{30005},1));
            Check(Method<AddFocus>(instance,39)(instance,IntPtr.Zero,this));
        }
        public void HandlePropertyChangedEvent(IntPtr sender,int id,object value) { Interlocked.Increment(ref Changes); if(id==30005 && value as string == "Unvisited descendant ready") Interlocked.Increment(ref DescendantChanges); if(value!=null&&Marshal.IsComObject(value)) Marshal.ReleaseComObject(value); }
        public void HandleFocusChangedEvent(IntPtr sender) { if(Name(sender)=="Unvisited descendant ready") Interlocked.Increment(ref FocusChanges); }
        public void Dispose()
        {
            if(subscribed!=null) { Check(Method<RemoveProperties>(instance,36)(instance,subscribed.Pointer,this)); Check(Method<RemoveFocus>(instance,40)(instance,this)); subscribed.Dispose(); subscribed=null; }
            Marshal.Release(walker); root.Dispose(); Marshal.Release(instance);
        }
        public static void Exercise(int process, bool hosted)
        {
            IntPtr main=Window(process,"UIA Review Fixture"); Require(main!=IntPtr.Zero,"fixture HWND");
            using(var opening=new Client(main)) opening.Invoke("Open lifetime");
            IntPtr hwnd=hosted?main:IntPtr.Zero;
            Wait(()=> {if(!hosted) hwnd=Window(process,"UIA Lifetime"); return hwnd!=IntPtr.Zero;},"lifetime HWND");
            var first=new Client(hwnd);
            using(var second=new Client(hwnd))
            {
                first.Listen("UIA Lifetime"); second.Listen("UIA Lifetime");
                first.Invoke("Change label"); Wait(()=>first.Changes>0&&second.Changes>0,"two property clients");
                int focus=second.FocusChanges; second.Invoke("New descendant");
                Wait(()=>first.DescendantChanges>0&&second.DescendantChanges>0,"event-only unvisited descendant name");
                uint foregroundProcess; GetWindowThreadProcessId(GetForegroundWindow(),out foregroundProcess);
                if(foregroundProcess==process) Wait(()=>second.FocusChanges>focus,"event-only unvisited descendant focus");
                else Console.WriteLine("UNVERIFIED desktop focus event: target is not foreground");
                using(var descendant=second.Find("Unvisited descendant ready")) { object focused; Check(Method<Property>(descendant.Pointer,10)(descendant.Pointer,30008,out focused)); Require((bool)focused,"new descendant logical focus"); }
                Exception releaseError=null;
                var release=new Thread(()=> {try {first.Dispose();} catch(Exception e) {releaseError=e;}}); release.SetApartmentState(ApartmentState.MTA); release.Start();
                using(var queried=second.Find("Unvisited descendant ready"))
                    for(int i=0;i<30;i++) { Require(Name(queried.Pointer)=="Unvisited descendant ready","query while other client releases"); Thread.Sleep(1); }
                Require(release.Join(10000),"off-thread release deadline"); if(releaseError!=null) throw releaseError;
                int changes=second.Changes; second.Invoke("Change label"); Wait(()=>second.Changes>changes,"remaining client's subscription");
                using(var old=second.Find("Lifetime document")) using(var range=Range(old)) using(var pattern=Pattern(old,10014,"32eba289-3583-42c9-9c59-3b6d9a1e9b6a"))
                {
                    Require(Contents(range)=="retained text","initial document"); string identity=Identity(old);
                    using(var same=second.Find("Lifetime document")) Require(Identity(same)==identity,"identity while represented");
                    using(var label=Pointer(old.Pointer,44)) Require(Name(label.Pointer).StartsWith("Lifetime label"),"provider-valued label");
                    ConcurrentRemoval(second,old,range); Unavailable(old,range);
                    IntPtr failed; Require(Method<GetPointer>(pattern.Pointer,7)(pattern.Pointer,out failed)==unchecked((int)0x80040201),"retired pattern unavailable");
                    second.Invoke("Reinsert"); using(var fresh=second.Find("Lifetime document")) Require(Identity(fresh)!=identity,"new identity after reinsertion");
                }
                using(var old=second.Find("Lifetime document")) using(var range=Range(old))
                {
                    second.Invoke("Move across windows"); Unavailable(old,range);
                    second.Invoke("Move back"); using(var fresh=second.Find("Lifetime document")) Require(RangeContents(fresh)=="retained text","cross-window reattachment");
                }
                using(var document=second.Find("Lifetime document")) using(var range=Range(document))
                {
                    second.Invoke("Replace model"); string text; Require(Method<Text>(range.Pointer,12)(range.Pointer,-1,out text)==unchecked((int)0x80040201),"model replacement invalidates range");
                    using(var fresh=Range(document)) Require(Contents(fresh)=="replacement text","replacement range");
                }
                using(var old=second.Find("Lifetime document")) using(var range=Range(old))
                {
                    second.Invoke("Replace template"); Unavailable(old,range);
                    using(var fresh=second.Find("Lifetime document")) using(var text=Range(fresh)) Require(Contents(text)=="replacement text","template content reattached");
                }
                // Release and reacquire without a control mutation; the infrastructure
                // may keep identity alive, so require usability rather than destruction.
                for(int i=0;i<10;i++) using(var document=second.Find("Lifetime document")) using(var text=Range(document)) Require(Contents(text)=="replacement text","reacquisition");
                second.Invoke("Deleting child");
                using(var childClient=new Client(hosted?main:WaitWindow(process,"Deleting lifetime")))
                using(var child=childClient.Find("Deleting lifetime"))
                using(var document=childClient.Search(child.Pointer,"Lifetime document"))
                using(var range=Range(document))
                using(var close=Pattern(child,10009,"0faef453-9208-43ef-bbb2-3b485177864f"))
                {
                    Check(Method<Action>(close.Pointer,3)(close.Pointer)); Unavailable(document,range);
                }
            }
            using(var retained=new Client(hwnd))
            using(var document=retained.Find("Lifetime document"))
            using(var range=Range(document))
            using(var closing=new Client(main))
            using(var root=closing.Find("UIA Review Fixture"))
            using(var close=Pattern(root,10009,"0faef453-9208-43ef-bbb2-3b485177864f"))
            {
                Check(Method<Action>(close.Pointer,3)(close.Pointer));
                Wait(()=> {object name;return Method<Property>(root.Pointer,10)(root.Pointer,30005,out name)<0;},"bounded shutdown disconnection");
                string text; Require(Method<Text>(range.Pointer,12)(range.Pointer,-1,out text)<0,"retained range disconnected after shutdown");
            }
        }
        static void ConcurrentRemoval(Client client, Reference element, Reference range)
        {
            Exception error=null;
            int retired=0;
            using(var started=new ManualResetEvent(false))
            using(var stop=new ManualResetEvent(false))
            {
                var reader=new Thread(()=> {
                    try {
                        while(!stop.WaitOne(0)) {
                            Marshal.AddRef(element.Pointer); Marshal.AddRef(range.Pointer);
                            try {
                                object name; string text;
                                int elementResult=Method<Property>(element.Pointer,10)(element.Pointer,30005,out name);
                                int rangeResult=Method<Text>(range.Pointer,12)(range.Pointer,-1,out text);
                                if(elementResult!=unchecked((int)0x80040201)) { Check(elementResult); if((string)name!="Lifetime document") throw new Exception("Concurrent element identity changed"); }
                                if(rangeResult!=unchecked((int)0x80040201)) { Check(rangeResult); if(text!="retained text") throw new Exception("Concurrent range text changed"); }
                                if(elementResult<0&&rangeResult<0) Interlocked.Increment(ref retired);
                            }
                            finally { Marshal.Release(range.Pointer); Marshal.Release(element.Pointer); }
                            started.Set(); Thread.Sleep(1);
                        }
                    } catch(Exception e) {error=e; started.Set();}
                });
                reader.SetApartmentState(ApartmentState.MTA); reader.IsBackground=true; reader.Start();
                try {
                    Require(started.WaitOne(10000),"concurrent reader starts");
                    if(error!=null) throw error;
                    client.Invoke("Detach");
                    Wait(()=>Volatile.Read(ref retired)>0||error!=null,"concurrent reader observes retirement");
                }
                finally { stop.Set(); Require(reader.Join(10000),"concurrent reader finishes"); }
                if(error!=null) throw error;
            }
        }
        // Read and release a temporary range in one operation.
        static string RangeContents(Reference element) { using(var range=Range(element)) return Contents(range); }
        static IntPtr WaitWindow(int process,string name) {IntPtr hwnd=IntPtr.Zero;Wait(()=> {hwnd=Window(process,name);return hwnd!=IntPtr.Zero;},name);return hwnd;}
    }
    public static void Run(int process,int port,bool hosted) { Require(Thread.CurrentThread.GetApartmentState()==ApartmentState.MTA,"MTA client"); Client.Exercise(process,hosted); Console.WriteLine("PASS native lifetime / "+assertions+" assertions / explicit COM release"); }
}
