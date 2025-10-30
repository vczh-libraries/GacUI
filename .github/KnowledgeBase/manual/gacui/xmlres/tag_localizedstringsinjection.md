# \<LocalizedStringsInjection\>

**\<LocalizedStringsInjection/\>** resource adds more languages to an existing **\<LocalizedStrings/\>**.

Its format is exactly like **\<LocalizedStrings/\>** but with the following differences: - There is no **DefaultLocale** attribute. - There is a **InjectIntoClassName** attribute, it should be the same value to the **ClassName** attribute of the **\<LocalizedStrings/\>** it adds to. The **ref.Class** attribute must be unique in the whole resource file just like others. Just find a name you like, it will not be used in any other part of the XML.

