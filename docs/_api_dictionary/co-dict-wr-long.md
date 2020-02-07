---
layout: article
title: CODictWrLong()
sidebar:
  nav: docs
---

This function writes a 32bit value to the given object dictionary.

<!--more-->

### Description

The object entry is addressed with the given key.

#### Prototype

```c
int16_t CODictWrLong(CO_DICT *cod, uint32_t key, uint32_t val);
```

#### Arguments

| Parameter | Description |
| --- | --- |
| cod | pointer to the object dictionary |
| key | object entry key; should be generated with `CO_DEV` |
| val | the source value |

#### Returned Value

- `== CO_ERR_NONE` : successful operation
- `!= CO_ERR_NONE` : an error is detected

### Example

The following example writes the value to the hypothetical application specific object entry "[1234:56]" within the object dictionary of the CANopen node AppNode.

```c
    int16_t  err;
    uint32_t  value = 0x30;
    :
    err = CODictWrLong (&(Appnode.Dict), CO_DEV(0x1234, 0x56), value);
    if (err != CO_ERR_NONE) {
        /* object [1234:56] is missing or error during writing */
    }
    :

```

Note: This function uses CODictFind() on each function call. To improve access performance for multiple accesses to a single object entry, the application may use CODictFind() once and COObjWrValue() multiple times.
