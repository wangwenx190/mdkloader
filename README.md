# mdkloader

Load MDK dynamically.

## Usage

```cpp
// Set the file name of MDK library.
// Can have absolute or relative path.
// Extension name is not needed.
mdkloader_setMdkLibName("mdk");
// Resolve MDK symbols.
mdkloader_initMdk();
// Judge whether MDK is loaded successfully or not.
Q_ASSERT(mdkloader_isMdkLoaded());
```
