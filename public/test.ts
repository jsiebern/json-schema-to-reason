// @ts-ignore
import json from './test.json';
import Converter from '../src_ts/index';

const res = Converter(JSON.stringify(json), 'Theme', {
    replaceRefs: [
        {
            re: /CSSProperties/m,
            replaceWith: 'ReactDOMRe.Style.t',
        },
        {
            re: /Partial.*([a-zA-Z]*)Props/m,
            replaceWith: 'Js.Json.t',
        },
        {
            re: /React\.>/m,
            replaceWith: 'Js.Json.t',
        },
        {
            re: /HTML.*Element/m,
            replaceWith: 'ReasonReact.reactElement',
        },
    ],
});

if (res) {
    console.log(res.modules);
    console.log(res.string);
    console.log(res.refmt);
}