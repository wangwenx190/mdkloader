# mdkloader

Load MDK dynamically.

## Usage

```cpp
// Set the file name of MDK library.
// Can have absolute or relative path.
// MDK will be loaded automatically.
mdkloader_load("mdk");
// Judge whether MDK is loaded successfully or not.
Q_ASSERT(mdkloader_isLoaded());
```
