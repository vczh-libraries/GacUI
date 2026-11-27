# GacUILibs -- Meta2DElement

## New Feature

## Fixed Bugs

## Milestone

**NOTE**: Put this in a GacUILibs organization.

- [Meta2DElement](./Unplanned/Graphics.md)
  - `Meta2DElement` will become an `IGuiGraphicsRawInterpretorFactory`.
  - Future 3d factory, chart factory and other factories could manually retrieve the `Meta2DElement` interpretor for internal use.
- Update KB pages about the new interpretor factory mechanism under raw element, with the below design philosophy:
  - The same interpretor is expected to have different implementation for each native renderer, but if redirect platform dependent implementation to existing interpretor is doable, it could be a universal implementation which internall requests a lower level interpretor for use.
