type ast;
[@bs.module "reason"] external parseRE: string => ast = "parseRE";

[@bs.module "reason"] external printRE: ast => string = "printRE";