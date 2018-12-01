import { JSONSchema7 } from 'json-schema';
import SchemaParser from './parsers/schema';

(async () => {
    const testData: JSONSchema7 = await import('./test.json');

    const parser = new SchemaParser(testData, 'Theme');
    const e = parser.getEntryParser();
    if (e) {
        e.parse();
    }

    console.log(parser.modules);
})();