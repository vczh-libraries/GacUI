# \<Doc\>

**\<Doc/\>**is a style-based rich document.

**\<Doc/\>**works like**\<Text/\>**, but there are a few differences:
- It can be assigned to properties that accept**presentation::DocumentModel^**.

## Syntax

Here is a sample of a**\<Doc/\>**resource item:
```
<Doc name="Description">
  <Doc>
    <Content>
      <p align="Center">
        <div style="Title">
          <b>TextEditor by GacUI</b>
        </div>
      </p>
      <p>
        <div style="Body">
          <nop>Welcome to the demo!</nop>
        </div>
      </p>
      <p>
        <div style="Body">
          <b>Author: </b>Zihan Chen (vczh)<br/>
          <b>Website: </b><a href="http://www.gaclib.net">http://www.gaclib.net</a><br/>
          <b>Github: </b><a href="https://github.com/vczh-libraries/Release">https://github.com/vczh-libraries/Release</a><br/>
        </div>
      </p>
    </Content>
    <Styles>
      <Style name="Content">
        <face>Segoe UI</face>
      </Style>
      <Style name="Title" parent="Content">
        <size>24</size>
        <b>true</b>
      </Style>
      <Style name="Body" parent="Content">
        <size>14</size>
      </Style>
    </Styles>
  </Doc>
</Doc>
```


There are two part of a document: styles and content.

### \<Styles\>

**\<Styles/\>**is optional.**\<Styles/\>**consists of zero to multiple**\<Style/\>**tags. A**\<Style/\>**looks like:
```
<Style name="NAME" parent="PARENT-STYLE (optional)">
  <face>FONT-NAME</face>
  <size>FONT-SIZE in pixel for 96 dpi</size>
  <color>TEXT-COLOR</color>
  <bkcolor>BACKGROUND-COLOR</bkcolor>
  <b>BOLD: true or false</b>
  <i>ITALIC: true or false</i>
  <u>UNDERLINE: true or false</u>
  <s>STRIKELINE: true or false</s>
  <antialias>ANTIALIAS: horizontal (default), vertical or no</antialias>
</Style>
```


**parent**is optional. When it exist, the style**NAME**will first copy all properties from**PARENT-STYLE**, and override what is specified in this tag.

Any document has five predefined styles, they always exist and cannot be specified in a document:
```
<Style name="#Default">
  <face>The default font face of the operator system</face>
  <face>The default font size of the operator system</face>
  <color>#000000</color>
  <bkcolor>#00000000</color>
  <b>false</b>
  <i>false</i>
  <u>false</u>
  <s>false</s>
  <antialias>default</antialias>
</Style>
<Style name="#Selection">
  <color>#FFFFFF</color>
  <bkcolor>#3399FF</bkcolor>
</Style>
<Style name="#Context">
</Style>
<Style name="#NormalLink" parent="#Context">
  <color>#0000FF</color>
  <u>true</u>
</Style>
<Style name="#ActiveLink" parent="#Context">
  <color>#FF8000</color>
  <u>true</u>
</Style>
```


If a document want to change a predefined style, let's say**#Default**, it should be written as an overriding:
```
<Style name="#Default-override">
  ...
</Style>
```
Overridings only work for predefined styles.

### \<Content\>

**\<Content/\>**consists of zero to multiple**\<p/\>**tags.**\<p/\>**has an optional attribute**align**, the value of which is one of**Left**,**Center**or**Right**. This property specify the alignment of the whole paragraph, the default alignment is**Left**.

**\<p/\>**consists of text. The syntax of text is:
```
TEXT ::= ELEMENT
     ::= TEXT ELEMENT

ELEMENT ::= Character                       // any valid XML character
        ::= <br/>                           // \r\n
        ::= <sp/>                           // a space character
        ::= <tab>                           // a tab character
        ::= <object name="NAME"/>           // embed a GacUI object here, will explain later
        ::= <img .../>                      // embed an image here, attributes are listed below
        ::= <font ...>TEXT</font>           // apply font to text, attributes are listed below
        ::= <a ...>TEXT</a>                 // make text to be a hyperlink, attributes are listed below
        ::= <b>TEXT</b>                     // bold
        ::= <i>TEXT</i>                     // italic
        ::= <u>TEXT</u>                     // underline
        ::= <s>TEXT</s>                     // strikeline
        ::= <ha>TEXT</ha>                   // horizontal antialias
        ::= <va>TEXT</va>                   // vertical antialias
        ::= <na>TEXT</na>                   // no antialias
        ::= <div style="STYLE">TEXT</div>   // apply style to text
        ::= <nop>TEXT</nop>                 // here it becomes TEXT, use it when it makes your XML looks clean to control spaces
        ::= <p>TEXT</p>                     // here it becomes TEXT, paragraph that is not in the top level will be ignored
```


Attributes of**\<img/\>**:
- **source**: URL of an image resource item, begins with**res://**or**import-res://**.
- **width**: (optional) override the width of the image.
- **height**: (optional) override the height of the image.
- **baseline**: (optional) override the baseline of the image.
- **frameIndex**: (optional) the frame index of the image, default to 0. Sometimes an image contains multiple frames.

Attributes of**\<font/\>**:
- **face**: (optional) name of the font.
- **size**: (optional) size of the font in pixel for 96 dpi.
- **color**: (optional) color of the text.
- **bkcolor**: (optional) background color of the text.

Attributes of**\<a/\>**:
- **href**: A hint of the hyperlink. When the hyperlink is clicked, this property will be passed to the control.
- **normal**: (optional) The normal style, default to**#NormalLink**.
- **active**: (optional) The active style, default to**#ActiveLink**.

## Applying Styles to Text

Control templates for**DocumentLabel**and**DocumentViewer**can also specify a default style. For example, here is**DarkSkin**'s default style for these two controls:
```
<Style name="#Default-Override">
    <color>#FFFFFF</color>
</Style>
<Style name="#Selection-Override">
    <color>#FFFFFF</color>
    <bkcolor>#007ACC</bkcolor>
</Style>
```


When a document is loaded to**DocumentLabel**or**DocumentViewer**, all fource default styles (font of the control, default styles of the control template, predefined and overriding) are merged together in the following way.

When we say**A + B**, it means we first use the style**A**, and then use all properties in**B**to override the previous result, and finally get a merged style.

Obviously, there is no difference between left-to-right or right-to-left association.

**C**,**T**,**D**stands for**control's Font property**,**control template**and**the document**. For example,**#Default[C]**means the**Font**property of the control, which contains everything except**color**and**bkcolor**.

**First round**: when a document is assigned to a control
- **#Default**= #Default[D] + #Default-override[D] + #Default[C] + #Default-override[T]
- **#Selection**= #Selection[D] + #Selection-override[D] + #Selection-override[T]
- **#Context**= #Context[D] + #Context-override[D] + #Context-override[T]
- **#NormalLink**= #NormalLink[D] + #NormalLink-override[D] + #NormalLink-override[T]
- **#ActiveLink**= #ActiveLink[D] + #ActiveLink-override[D] + #ActiveLink-override[T]We will see that, the configuration in the control and the control template has a higher priority. If a document want to apply styles in some text, it should specify it explicitly (see**\<Content\>**for details) instead of trying to override predefined styles.

**Second round**: when a document is displayed:
- **normal text**: #Default
- **hyperlink**: #Default + #NormalLink
- **active hyperlink**: #Default + #ActiveLink
- **selected normal text**: #Default + #Selection
- **selected hyperlink**: #Default + #NormalLink + #Selection
- **selected active hyperlink**: #Default + #ActiveLink + #SelectionActive hyperlink is a hyperlink that is under the mouse cursor. A hyperlink can specify its own normal style and active style, but if they are not specified, they become**#NormalLink**and**#ActiveLink**

## Space Characters

When there are spaces between tags,
- If there are other characters, all spaces (including leading and trailing) are preserved.
- If there are only space characters, they are all ignored.

For example, in this paragraph:
```
<p>
    <nop>This is a book.<br/></nop>
    <nop>It is about GacUI!<br/></nop>
</p>
```
The text is**This is a book.\r\nIt is about GacUI!\r\n**.

For example, but in this paragraph:
```
<p>
    This is a book.
    It is about GacUI!
</p>
```
The text is**\r\n[many-spaces]This is a book.\r\n[many-spaces]It is about GacUI!\r\n[many-spaces]**.

The**\<nop/\>...\<br/\>\<nop/\>**pattern is highly recommended to specify lines in paragraphs. If there is only one line, just skip \<br/\>.

## Sample Application

[DocumentEditor](https://github.com/vczh-libraries/Release/tree/master/Tutorial/GacUI_Controls/DocumentEditor),[DocumentEditorRibbon](https://github.com/vczh-libraries/Release/tree/master/Tutorial/GacUI_Controls/DocumentEditorRibbon)and[DocumentEditorToolstrip](https://github.com/vczh-libraries/Release/tree/master/Tutorial/GacUI_Controls/DocumentEditorToolstrip)are document editors in toolstrip and ribbon.

Here we created another sample for[hyperlink and embedded controls](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/kb_xmlres_data/DocPage1.xml):


- Source code:[kb_xmlres_data](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/kb_xmlres_data/DocPage1.xml)
- ![](https://gaclib.net/doc/gacui/kb_xmlres_tag_doc.gif)

