# Instance Inheriting

An **\<Instance/\>** can be a base type of another **\<Instance/\>**. These two instances are required to be in the same XML resource, or the base type is required to be in [one of the dependency](../../.././gacui/xmlres/cxrr.md) specified in [the Metadata configuration](../../.././gacui/xmlres/cgc.md).

Typically, you don't want your **\<Instance/\>** to be in the default namespaces, so it is very common to use [xmlns:NAMESPACE](../../.././gacui/xmlres/instance/namespaces.md) for the base type like: ``` <Resource> <Instance name="BaseInstanceResource"> <Instance ref.Class="sample::MyBaseInstance"> <Window/> </Instance> </Instance> <Instance name="DerivedInstanceResource"> <Instance ref.Class="sample::MyDerivedInstance" xmlns:sample="sample::*"> <sample:MyBaseInstance/> </Instance> </Instance> </Resource> ```

Please check out [Cross XML Resource References](../../.././gacui/advanced/cxrr.md) for details.

