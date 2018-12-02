import { JSONSchema7 } from 'json-schema';
import SchemaParser from './parsers/schema';

(async () => {
    const testData: JSONSchema7 = await import('./test.json');

    const parser = new SchemaParser(testData, 'Theme', {
        replaceRefs: [
            {
                re: /CSSProperties/m,
                replaceWith: 'ReactDOMRe.Style.t',
            },
            {
                re: /Partial.*([a-zA-Z]*)Props/m,
                replaceWith: 'Js.t({..})',
            },
            {
                re: /React\.>/m,
                replaceWith: 'Js.t({..})',
            },
        ],
    });
    const e = parser.getEntryParser();
    if (e) {
        e.parse();
    }

    console.log(parser.modules);
})();