open Migrate_parsetree;
open Ast_408;
open Ast_helper;
open Ast_mapper;
open Asttypes;
open Parsetree;
open Longident;

module Filename = {
  let is_dir_sep = (s, i) => s.[i] == '/';

  let extension_len = name => {
    let rec check = (i0, i) =>
      if (i < 0 || is_dir_sep(name, i)) {
        0;
      } else if (name.[i] == '.') {
        check(i0, i - 1);
      } else {
        String.length(name) - i0;
      };
    let rec search_dot = i =>
      if (i < 0 || is_dir_sep(name, i)) {
        0;
      } else if (name.[i] == '.') {
        check(i, i - 1);
      } else {
        search_dot(i - 1);
      };
    search_dot(String.length(name) - 1);
  };
  let extension = name => {
    let l = extension_len(name);
    if (l == 0) {
      "";
    } else {
      String.sub(name, String.length(name) - l, l);
    };
  };
  let chop_extension = name => {
    let l = extension_len(name);
    if (l == 0) {
      invalid_arg("Filename.chop_extension");
    } else {
      String.sub(name, 0, String.length(name) - l);
    };
  };
  let remove_extension = name => {
    let l = extension_len(name);
    if (l == 0) {
      name;
    } else {
      String.sub(name, 0, String.length(name) - l);
    };
  };
};

let refactorMapper = {
  ...default_mapper,
  expr: (mapper, expr) => {
    switch (expr) {
    | {
        pexp_desc:
          Pexp_apply(
            {
              pexp_desc:
                Pexp_ident({
                  loc,
                  txt: Ldot(Ldot(Lident("React"), "Ref"), "current"),
                }),
            },
            [(_arg, exp)],
          ),
      }
    | {
        pexp_desc:
          Pexp_apply(
            {pexp_desc: Pexp_ident({loc, txt: Lident("|.")})},
            [
              (_arg, exp),
              (
                _,
                {
                  pexp_desc:
                    Pexp_ident({
                      txt: Ldot(Ldot(Lident("React"), "Ref"), "current"),
                    }),
                },
              ),
            ],
          ),
      } =>
      Exp.field(mapper.expr(mapper, exp), {loc, txt: Ldot(Lident("React"), "current")})
    | {
        pexp_desc:
          Pexp_apply(
            {
              pexp_desc:
                Pexp_apply(
                  {pexp_desc: Pexp_ident({loc, txt: Lident("|.")})},
                  [
                    (_, exp),
                    (
                      _,
                      {
                        pexp_desc:
                          Pexp_ident({
                            txt:
                              Ldot(
                                Ldot(Lident("React"), "Ref"),
                                "setCurrent",
                              ),
                          }),
                      },
                    ),
                  ],
                ),
            },
            [(_, thingToApply)],
          ),
      }
    | {
        pexp_desc:
          Pexp_apply(
            {
              pexp_desc:
                Pexp_ident({
                  loc,
                  txt: Ldot(Ldot(Lident("React"), "Ref"), "setCurrent"),
                }),
            },
            [(_, exp), (_, thingToApply)],
          ),
      } =>
      Exp.setfield(
        mapper.expr(mapper, exp),
        {loc, txt: Ldot(Lident("React"), "current")},
        mapper.expr(mapper, thingToApply),
      )
    | {
        pexp_desc:
          Pexp_apply(
            exp,
            args,
          ),
        pexp_attributes: [{attr_name: {txt: "JSX"}}]
      } =>
      {...expr, pexp_desc: Pexp_apply(mapper.expr(mapper, exp), List.map(((label, arg)) =>
        switch (label) {
          | Labelled("crossorigin") => (Labelled("crossOrigin"), mapper.expr(mapper, arg))
          | Labelled("min") => (
            Labelled("min"), 
            Exp.apply(
              Exp.ident({loc: default_loc^, txt: Lident("string_of_int")}),
              [(Nolabel, mapper.expr(mapper, arg))],
            )
          )
          | _ => (label, mapper.expr(mapper, arg))
        }, args))}
    | expr =>
      default_mapper.expr(mapper, expr)
    };
  },
  typ: (mapper, typ) => {
    switch (typ) {
    | {
        ptyp_desc:
          Ptyp_constr(
            {loc, txt: Ldot(Ldot(Lident("React"), "Ref"), "t")},
            expList,
          ),
      } =>
      Typ.constr({loc, txt: Ldot(Lident("React"), "ref")}, expList)
    | typ =>
      default_mapper.typ(mapper, typ)
    };
  },
};

module StringSet = Set.Make(String);

let read = () => {
  let set = ref(StringSet.empty);
  let rec read = () =>
    try(
      {
        set := set^ |> StringSet.add(stdin |> input_line);
        read();
      }
    ) {
    | End_of_file => ()
    };
  read();
  set^;
};

let main = () =>
  switch (Sys.argv) {
  | [||]
  | [|"help" | "-help" | "--help"|] =>
    print_endline("upgrade-reason-react");
    print_endline("Helps you migrate ReasonReact from 0.6 to 0.7");
    print_endline("Usage: find src/**/*.re | migrate");
    print_endline("Usage: pass a list of .re files you'd like to convert.");
  | args =>
    read()
    |> StringSet.filter(item => Filename.extension(item) == ".re")
    /* Uncomment next line for debug */
    /* && ! String.contains(item, '_') */
    |> StringSet.iter(fileName =>
         try (
           {
             let outputDir =
               args |> Array.exists(item => item == "--demo") ? "output/" : "";
             let file = fileName |> Filename.remove_extension;
             let ic = open_in_bin(file ++ ".re");
             let lexbuf = Lexing.from_channel(ic);
             let (ast, comments) =
               Reason_toolchain.RE.implementation_with_comments(
                 lexbuf,
               );
             let newAst =
               refactorMapper.structure(
                 refactorMapper,
                 ast,
               );
             let target = outputDir ++ file ++ ".re";
             let oc = open_out_bin(target);
             if (Sys.file_exists(file ++ ".rei")) {
               let ic = open_in_bin(file ++ ".rei");
               let lexbuf = Lexing.from_channel(ic);
               let (ast, comments) =
                 Reason_toolchain.RE.interface_with_comments(
                   lexbuf,
                 );
               let newAst =
                 refactorMapper.signature(
                   refactorMapper,
                   ast,
                 );
               let target = outputDir ++ file ++ ".rei";
               let oc = open_out_bin(target);
               let formatter = Format.formatter_of_out_channel(oc);
               Reason_toolchain.RE.print_interface_with_comments(
                 formatter,
                 (newAst, comments),
               );
               Format.print_flush();
               print_endline({js|✅ Done |js} ++ target);
               close_out(oc);
             };
             let formatter = Format.formatter_of_out_channel(oc);
             Reason_toolchain.RE.print_implementation_with_comments(
               formatter,
               (newAst, comments),
             );
             Format.print_flush();
             print_endline({js|✅ Done |js} ++ target);
             close_out(oc);
           }
         ) {
         | _ =>
           let outputDir =
             args |> Array.exists(item => item == "--demo") ? "output/" : "";
           let file = fileName |> Filename.remove_extension;
           let target = outputDir ++ file ++ ".re";
           print_endline({js|❗️️ Errored on |js} ++ target);
         }
       );
    print_endline("Done!");
  };

main();
