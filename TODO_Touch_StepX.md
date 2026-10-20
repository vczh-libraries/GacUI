Details are in [TODO_Touch.md](./TODO_Touch.md), but this document has a much bigger scope, so keep the target scope in this request, uncovered parts will be handled in future requests. Here are completed works:
- `TODO_Touch_Step1.md`
- `TODO_Touch_Step2.md`
- `TODO_Touch_Step3.md`
- `TODO_Touch_StepR1.md`
- `TODO_Touch_StepR2.md`

There is a known issue when handling mouse events on a composition, the handler deletes the composition, and GacUI will crash. This is expected, and point events will be handled just like how mouse events are handled, it will have the same issue, and this is also expected. When implementing point events, you are going to figure out if any code for mouse events could be reused for propogating and capturing. GacUI will offer a pair of new capture/release functions for point events, they work like how captures are done to mouse events.
