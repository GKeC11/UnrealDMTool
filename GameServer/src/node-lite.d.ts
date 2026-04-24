declare const require: any;
declare const process: any;
declare const __dirname: string;

declare module "node:crypto" {
  const value: any;
  export = value;
}

declare module "node:child_process" {
  const value: any;
  export = value;
}

declare module "node:fs" {
  const value: any;
  export = value;
}

declare module "node:http" {
  const value: any;
  export = value;
}

declare module "node:path" {
  const value: any;
  export = value;
}

declare module "node:url" {
  export const URL: any;
}
