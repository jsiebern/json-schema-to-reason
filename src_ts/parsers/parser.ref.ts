import BaseParser from './_base';
import getParser from './_getParser';

import { JSONSchema7Definition } from 'json-schema';
import SchemaParser from './schema';

class RefParser extends BaseParser {
    public constructor(schema: SchemaParser, key: string, def: JSONSchema7Definition) {
        super(schema, key, def);
    }

    public parse(): void {
        throw new Error("Method not implemented.");
    }
    public getReasonType(): string {
        throw new Error("Method not implemented.");
    }
}

export default RefParser;