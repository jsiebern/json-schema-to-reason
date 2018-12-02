import BaseParser from './_base';
import getParser from './_getParser';

let i = 0;

class ObjectParser extends BaseParser {
    private properties: BaseParser[] = [];

    public parse() {
        if (typeof this.def === 'boolean') {
            return;
        }
        if (!this.def.properties) {
            return;
        }
        if (this.schema.hasModule(this.key)) {
            return;
        }

        const properties = this.def.properties;
        Object.keys(properties).forEach(key => {
            const parser = getParser(this.schema, properties[key]);
            if (!parser) {
                return;
            }
            i = i + 1;
            if (i > 10000) {
                throw 'FUCK';
            }
            this.properties.push(new parser(this.schema, key, properties[key]));
        });
        this.properties.forEach(p => p.parse());

        this.schema.modules[this.key] = {
            name: this.key,
            properties: this.properties.map(property => ({
                name: property.key,
                optional: false,
                reasonType: property.getReasonType(),
            }))
        };
    }

    public getReasonType() {
        if (this.properties.length) {
            return `${this.key}.t`;
        }
        else {
            return 'Js.t({..})';
        }
    }

}

export default ObjectParser;