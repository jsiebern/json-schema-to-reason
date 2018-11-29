module Res = Belt.Result;

module Utils = {
  let cleanOptionArray = arr =>
    arr
    ->Belt.Array.keep(v => v->Belt.Option.isSome)
    ->Belt.Array.map(v => v->Belt.Option.getExn);
};

type mixedEnum = [
  | `String(string)
  | `Int(int)
  | `Float(float)
  | `Bool(bool)
]
and jsonSchema = {
  schema: string,
  id: option(string),
  title: option(string),
  definitions: option(jsonDefinitions),
  properties: option(jsonDefinitions),
  default: option(string),
}
and jsonArray = {items: array(jsonType)}
and jsonObject = {
  required: option(array(string)),
  properties: jsonDefinitions,
}
and jsonDefinitions = Belt.Map.String.t(jsonType)
and jsonType =
  | Schema(jsonSchema)
  | Float
  | Int
  | String
  | Bool
  | Object(jsonObject)
  | Array(jsonArray)
  | Null
  | Ref(string)
  | Union(array(jsonType))
  | StringEnum(array(string))
  | MixedEnum(array(mixedEnum))
  | AllOf(array(jsonType))
  | AnyOf(array(jsonType))
  | OneOf(array(jsonType))
  | Not(jsonType);

module D = Decode.AsOption;

module Make = {
  let jsonSchema = (schema, id, title, definitions, properties, default) => {
    schema,
    id,
    title,
    definitions,
    properties,
    default,
  };

  let jsonArray = items => {items: items};

  let jsonObject = (required, properties) => {required, properties};
};

let rec decodeDefinitions = json =>
  switch (json |> Js.Json.decodeObject) {
  | Some(obj) =>
    Some(
      obj
      ->Js.Dict.keys
      ->Belt.Array.map(key =>
          (key, obj->Js.Dict.unsafeGet(key)->decodeJsonType)
        )
      ->Belt.Array.keep(t => t->snd->Belt.Option.isSome)
      ->Belt.Array.map(t => (t->fst, t->snd->Belt.Option.getExn))
      ->Belt.Map.String.fromArray,
    )
  | None => None
  }
and decodeJsonObject = json =>
  Some(
    D.Pipeline.(
      succeed(Make.jsonObject)
      |> optionalField("required", D.array(D.string))
      |> field("properties", decodeDefinitions)
      |> run(json)
    )
    ->Belt.Option.getWithDefault({
        required: None,
        properties: Belt.Map.String.empty,
      }),
  )
and decodeJsonSchema = json =>
  D.Pipeline.(
    succeed(Make.jsonSchema)
    |> field("schema", D.string)
    |> field("id", D.(optional(string)))
    |> field("title", D.(optional(string)))
    |> field("definitions", D.optional(decodeDefinitions))
    |> field("properties", D.optional(decodeDefinitions))
    |> field("default", D.(optional(string)))
    |> run(json)
  )
and decodeJsonArray = json =>
  Make.jsonArray(
    (json |> D.array(decodeJsonType))->Belt.Option.getWithDefault([||]),
  )
and decodeMixedEnum = json =>
  switch (json |> D.string, json |> D.int, json |> D.float, json |> D.boolean) {
  | (Some(str), _, _, _) => Some(`String(str))
  | (_, Some(int), _, _) => Some(`Int(int))
  | (_, _, Some(float), _) => Some(`Float(float))
  | (_, _, _, Some(bool)) => Some(`Bool(bool))
  | _ => None
  }
and decodeJsonType = json =>
  switch (
    json |> D.field("$schema", D.string),
    json |> D.field("type", D.string),
    json |> D.field("type", D.array(D.string)),
  ) {
  | (Some(_), _, _) =>
    switch (json |> decodeJsonSchema) {
    | Some(schema) => Some(Schema(schema))
    | None => None
    }
  | (None, Some(t), None) =>
    switch (t) {
    | "number" => Some(Float)
    | "integer" => Some(Int)
    | "null" => Some(Null)
    | "boolean" => Some(Bool)
    | "string" =>
      switch (json |> D.field("enum", D.array(D.string))) {
      | None => Some(String)
      | Some(strEnum) => Some(StringEnum(strEnum))
      }
    | "array" => Some(Array(json |> decodeJsonArray))
    | "object" => (json |> decodeJsonObject)->Belt.Option.map(o => Object(o))
    | _ => None
    }
  | (None, None, Some(arr)) =>
    Some(
      Union(
        arr
        ->Belt.Array.map(typeName =>
            Js.Dict.fromList([("type", Js.Json.string(typeName))])
            |> Js.Json.object_
            |> decodeJsonType
          )
        ->Utils.cleanOptionArray,
      ),
    )
  | _ =>
    switch (json |> D.field("$ref", D.string)) {
    | Some(refStr) => Some(Ref(refStr))
    | None =>
      switch (
        json |> D.field("allOf", D.array(decodeJsonType)),
        json |> D.field("anyOf", D.array(decodeJsonType)),
        json |> D.field("oneOf", D.array(decodeJsonType)),
        json |> D.field("not", decodeJsonType),
      ) {
      | (Some(allOf), _, _, _) => Some(AllOf(allOf))
      | (_, Some(anyOf), _, _) => Some(AnyOf(anyOf))
      | (_, _, Some(oneOf), _) => Some(OneOf(oneOf))
      | (_, _, _, Some(not)) => Some(Not(not))
      | _ =>
        switch (json |> D.field("enum", D.array(decodeMixedEnum))) {
        | Some(mixedArray) => Some(MixedEnum(mixedArray))
        | None =>
          Js.log2("ERROR", json);
          None;
        }
      }
    }
  };