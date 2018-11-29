[%%debugger.chrome];

external cAny: 'a => 'b = "%identity";

let json = Test.json;

let t = json->Types.decodeJsonType;
Js.log2("RESULT: ", t);