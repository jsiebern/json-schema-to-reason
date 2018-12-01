import { JSONSchema7Definition } from 'json-schema';
import SchemaParser from './schema';

abstract class BaseParser {
    protected readonly schema: SchemaParser;
    protected readonly def: JSONSchema7Definition;
    public readonly key: string;

    public constructor(schema: SchemaParser, key: string, def: JSONSchema7Definition) {
        this.key = key;
        this.schema = schema;
        this.def = def;
    }

    public abstract parse(): void;

    public abstract getReasonType(): string;
}

export default BaseParser;