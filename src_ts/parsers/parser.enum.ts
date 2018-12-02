import BaseParser from './_base';

class EnumParser extends BaseParser {
    public parse(): void {
        if (typeof this.def === 'boolean') {
            return;
        }
        if (!this.def.enum) {
            return;
        }

    }

    public getReasonType(): string {
        return 'bool';
    }
}

export default EnumParser;