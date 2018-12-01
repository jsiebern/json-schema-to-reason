open Types;
module Res = Belt.Result;
module BMap = Belt.Map.String;

type property = {
  name: string,
  rName: string,
  rType: string,
};

let parse =
    (
      ~typeName: string,
      ~jsonType: jsonType,
      ~resolveModulesAs: BMap.t(string),
    ) => {
  let renderedModules = ref(Belt.Set.String.empty);

  let getRefName = jsonRef =>
    jsonRef |> Js.String.replace("#/definitions/", "");

  let resolveRef = (~jsonRef: jsonRef, ~definitions: jsonDefinitions) => {
    let refKey = jsonRef->getRefName;
    let matches = [%bs.re "/[<,]([a-zA-Z_]*)[,>]/gm"] |> Js.Re.exec(refKey);
    let moduleName =
      switch (matches) {
      | Some(m) =>
        switch (m->Js.Re.captures->Belt.Array.get(1)) {
        | Some(m) =>
          switch (m->Js.Nullable.toOption) {
          | Some(m) => m
          | None => refKey
          }
        | None => refKey
        }
      | None => refKey
      };
    (moduleName, definitions->BMap.get(refKey));
  };

  let propertyToModuleName = v => v;
  let capitalize = v => v;
  let toReasonName = v => v;

  let resolveTypeName =
      (~key: string, ~jsonType: jsonType, ~definitions: jsonDefinitions) =>
    switch (jsonType) {
    | String => "string"
    | Int => "int"
    | Float => "float"
    | Bool => "bool"
    | Ref(jsonRef) =>
      let r = resolveRef(~jsonRef, ~definitions)->fst;
      {j|$r.t|j};
    | Object(_) => "Js.t({..})"
    | StringEnum(_enumArray) =>
      let moduleName = key->propertyToModuleName;
      {j|
          $moduleName.t
        |j};
    | _ =>
      Js.log2("OH_BOY", jsonType);
      "OH_BOY";
    };

  let i = ref(0);

  let rec renderTypeModules =
          (
            ~key: string,
            ~definitions: jsonDefinitions,
            ~jsonType: jsonType,
            ~resolveModulesAs: BMap.t(string),
          ) =>
    switch (jsonType) {
    | Object(obj)
        when
          !(renderedModules^)->Belt.Set.String.has(key)
          && !obj.properties->BMap.isEmpty =>
      renderedModules := (renderedModules^)->Belt.Set.String.add(key);
      renderObject(~name=key, ~t=obj, ~definitions, ~resolveModulesAs);
    | StringEnum(enumArray) =>
      let moduleName = key->propertyToModuleName;
      let items =
        enumArray
        ->Belt.Array.map(value => {
            let capitalized = value->capitalize->toReasonName;
            {j|| [@bs.as "$value"] `$capitalized|j};
          })
        ->Js.Array.joinWith("\n", _);
      {j|
          module $moduleName {
            [@bs.deriving jsConverter]
            type t = [
              $items
            ];
          }
        |j};
    | Ref(jsonRef) =>
      let refName = key->getRefName;
      switch (resolveModulesAs->BMap.get(refName)) {
      | Some(_) => ""
      | None =>
        let resolvedRef = resolveRef(~jsonRef, ~definitions);
        switch (resolvedRef->snd) {
        | Some(jsonType) when i^ < 10000 =>
          i := i^ + 1;
          renderTypeModules(
            ~key=resolvedRef->fst,
            ~jsonType,
            ~definitions,
            ~resolveModulesAs,
          );
        | Some(jsonType) =>
          Js.log2(
            "_____________________________________OH_SHIT_LEVEL_10000",
            jsonType,
          );
          "";
        | None => ""
        };
      };
    | _ => ""
    }
  and renderObject =
      (
        ~name: string,
        ~t: jsonObject,
        ~definitions: jsonDefinitions,
        ~resolveModulesAs: BMap.t(string),
      ) => {
    /* Extract objects to render from properties */
    let properties =
      t.properties
      ->BMap.mapWithKey((key, property) =>
          {
            name: key,
            rName: key,
            rType: resolveTypeName(~key, ~jsonType=property, ~definitions),
          }
        )
      ->BMap.toArray
      ->Belt.Array.map(v => v->snd);

    let typesBefore =
      t.properties
      ->BMap.toArray
      ->Belt.Array.map(((key, property)) =>
          renderTypeModules(
            ~key,
            ~definitions,
            ~jsonType=property,
            ~resolveModulesAs: BMap.t(string),
          )
        )
      ->Belt.Array.keep(v => v !== "")
      ->Js.Array.joinWith("\n", _);

    let sProperties =
      properties
      ->Belt.Array.map(({rName, name, rType}) =>
          {j|[@bs.as "$name"] $rName: $rType|j}
        )
      ->Js.Array.joinWith(",\n", _);

    let genericObjectSplit = sProperties->Js.String.split("{..}");

    let sProperties =
      genericObjectSplit->Belt.Array.size > 1 ?
        genericObjectSplit
        ->Belt.Array.mapWithIndex((i, str) =>
            i mod 2 === 0 ?
              str : str ++ "'" ++ String.make(1, (i - 1)->char_of_int)
          )
        ->Js.Array.join :
        sProperties;

    {j|
      $typesBefore

      module $name {
        [@bs.deriving abstract]
        type t = {
          $sProperties
        };
      }
    |j};
  };

  switch (jsonType) {
  | Schema({definitions: Some(definitions)}) =>
    switch (definitions->Belt.Map.String.get(typeName)) {
    | Some(Object(t)) =>
      Res.Ok(
        renderObject(~name=typeName, ~t, ~definitions, ~resolveModulesAs),
      )
    | Some(_) => Res.Error(typeName ++ " is not an object")
    | None => Res.Error(typeName ++ " not found in definitions")
    }
  | _ => Res.Error("Entry point is not a schema")
  };
};