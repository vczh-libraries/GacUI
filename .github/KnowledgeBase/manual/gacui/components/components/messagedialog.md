# \<MessageDialog\>

**\<MessageDialog/\>** displays a OS native message dialog.

## Input Properties

Set these properties before calling **ShowDialog** to define the content of the dialog.

### Input

The value of this property must be one of the followuing values: - **DisplayOK**: Display OK button. - **DisplayOKCancel**: Display OK / Cancel button. - **DisplayYesNo**: Display Yes / No button. - **DisplayYesNoCancel**: Display Yes / No / Cancel button. - **DisplayRetryCancel**: Display Retry / Cancel button. - **DisplayAbortRetryIgnore**: Display Abort / Retry / Ignore button. - **DisplayCancelTryAgainContinue**: Display Cancel / Try Again / Continue button.

### DefaultButton

The value of this property must be one of the followuing values: - **DefaultFirst**: The first button is focused right after the dialog is displayed. - **DefaultSecond**: The second button is focused right after the dialog is displayed. - **DefaultThird**: The third button is focused right after the dialog is displayed.

### Icon

The value of this property must be one of the followuing values: - **IconNone**: Do not display an icon. - **IconError**: Display an error icon. - **IconQuestion**: Display a question icon. - **IconWarning**: Display a warning icon. - **IconInformation**: Display an information icon.

### ModalOption

The value of this property must be one of the followuing values: - **ModalWindow**: The current window is disabled until the dialog is closed. - **ModalTask**: All window of the current application is disabled until the dialog is closed. - **ModalSystem**: Same as **ModelTask** and the dialog is set to top-most.

### Text

This property defines the content of the dialog.

### Title

This property defines the title of the dialog.

## ShowDialog Method

Call this function to display a dialog, and returns the button that is clicked to close the dialog: - **SelectOK**: The OK button is clicked. - **SelectCancel**: The Cancel button is clicked. - **SelectYes**: The Yes button is clicked. - **SelectNo**: The No button is clicked. - **SelectRetry**: The Retry button is clicked. - **SelectAbort**: The Abort button is clicked. - **SelectIgnore**: The Ignore button is clicked. - **SelectTryAgain**: The Try Again button is clicked. - **SelectContinue**: The Continue button is clicked.

