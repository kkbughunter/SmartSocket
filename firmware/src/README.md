
- **Send Data to Firebase**
```cpp
if (millis() - ms > 20000 && app.ready())
  {
    ms = millis();

    JsonWriter writer;
    object_t json, obj1, obj2;

    writer.create(obj1, "ms", ms);
    writer.create(obj2, "rand", random(10000, 30000));
    writer.join(json, 2, obj1, obj2);

    Database.set<object_t>(aClient2, "/test/stream/number", json, asyncCB, "setTask");
  }
  ```