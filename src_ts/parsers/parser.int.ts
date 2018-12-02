import BaseParser from './_base';

class IntParser extends BaseParser {
    public parse(): void {

    }
    public getReasonType(): string {
        return 'int';
    }
}

export default IntParser;