import BaseParser, { Parser } from './_base';

import { capitalize, generateAttributeName } from '../helpers';
import getParser from './_getParser';

class UnionParser extends BaseParser {
    private types: (Parser | false)[];

    public parse(): void {
        if (typeof this.def === 'boolean') {
            return;
        }
        if (!Array.isArray(this.def.type)) {
            return;
        }

        this.types = this.def.type.map(t => {
            const parser = getParser(this.schema, { type: t });
            if (typeof parser !== 'boolean') {
                return new parser(this.schema, "", { type: t });
            }
            return false;
        }).filter(p => typeof p !== 'boolean');
    }
    public getReasonType(): string {
        return 'string';
    }

    public render() {
        return '';
    }

    public getGetterFunc(optional: boolean) {
        const attributeName = generateAttributeName(this.key);
        return `let ${attributeName}Get = (value) => {
            let isNumeric: 'a => bool = [%raw {|
                function(obj) {
                    var realStringObj = obj && obj.toString();
                    return typeof obj !== 'object' && (realStringObj - parseFloat(realStringObj) + 1) >= 0;
                }
            |}];
            let isBool: 'a => bool = [%raw {|
                function(obj) {
                    return typeof obj === 'boolean';
                }
            |}];
            ${optional ? `
                ${attributeName}Get(value)->Belt.Option.map(v => isNumeric(v) ? \`Float(Obj.magic(v)) : isBool(v) ? \`Bool(Obj.magic(v)) : \`String(Obj.magic(v)));
            ` : `
            let v = ${attributeName}Get(value);
            isNumeric(v) ? \`Float(Obj.magic(v)) : isBool(v) ? \`Bool(Obj.magic(v)) : \`String(Obj.magic(v));
            `}
        };`;
    }
}

export default UnionParser;