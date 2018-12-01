import { JSONSchema7, JSONSchema7Definition } from 'json-schema';
import getParser from './_getParser';

class SchemaParser {
    private readonly _schema: JSONSchema7;
    private readonly _entry: string;
    private readonly _definitions: { [name: string]: JSONSchema7Definition } | undefined[];

    public modules: { [name: string]: moduleDefinition } = {};

    public hasModule = (name: string) => typeof this.modules[name] !== 'undefined';

    public getDefinition(defName: string): JSONSchema7Definition | undefined {
        return this._definitions[defName.replace('#/definitions/', '')];
    }

    public constructor(schema: JSONSchema7, entry: string) {
        this._entry = entry;
        this._schema = schema;
        this._definitions = (this._schema.definitions != null) ? this._schema.definitions : [];
    }

    public getEntryParser() {
        const entry = this._definitions[this._entry];
        const parser = getParser(this, entry);

        if (parser) {
            return new parser(this, this._entry, entry);
        }
        return false;
    }
}

export default SchemaParser;