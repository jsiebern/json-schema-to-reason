import BaseParser, { Parser } from './_base';

import { capitalize } from '../helpers';
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
        if (!this.types) {
            console.log(`${this.key}: Unsupported union types?`);
            return '__ERROR__';
        }
        return `[
            ${this.types.map(t => t ? `| \`${capitalize(t.getReasonType())}(${t.getReasonType()})` : '').join('\n')}
        ]`;
    }

    public render() {
        return '';
    }
}

export default UnionParser;