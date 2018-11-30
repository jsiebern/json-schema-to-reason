[%%debugger.chrome];

external cAny: 'a => 'b = "%identity";

let json = Test.json;

let t = json->Types.decodeJsonType;
Js.log2("RESULT: ", t);

module Comp = {
  let component = ReasonReact.statelessComponent(__MODULE__);
  let make = _ => {
    ...component,
    render: _ =>
      switch (t) {
      | Some(t) =>
        <div
          style={
            ReactDOMRe.Style.make(
              ~width="80%",
              ~margin="0 auto",
              ~height="100%",
              ~fontFamily="Courier New",
              ~padding="30px",
              (),
            )
          }>
          <pre>
            {
              switch (
                Parser.parse(
                  ~typeName="Theme",
                  ~jsonType=t,
                  ~resolveModulesAs=
                    [|("CSSProperties", "ReactDOMRe.Style.t")|]
                    ->Belt.Map.String.fromArray,
                )
              ) {
              | Belt.Result.Ok(str)
              | Belt.Result.Error(str) => str->ReasonReact.string
              }
            }
          </pre>
        </div>
      | None => "NOOOOO"->ReasonReact.string
      },
  };
};

ReactDOMRe.renderToElementWithId(<Comp />, "app");