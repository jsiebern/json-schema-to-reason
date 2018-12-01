import { JSONSchema7Definition } from 'json-schema';

import Schema from './schema';
import ObjectParser from './parser.object';

const getParser = (schema: Schema, def: JSONSchema7Definition): false | typeof ObjectParser => {
    if (typeof def !== 'boolean') {
        if (def.type != null) {
            if (def.type === 'object') {
                return ObjectParser;
            }
        }
        else if (def.$ref != null) {
            const ref = schema.getDefinition(def.$ref);
            return typeof ref !== 'undefined' && typeof ref !== 'boolean' ? getParser(schema, ref) : false;
        }
    }
    return false;
};

export default getParser;