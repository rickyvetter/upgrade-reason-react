## ReasonReact migration script

**Experimental, use with caution!**

### Usage

```console
find src -name "*.re" | ./node_modules/upgrade-reason-react/lib/bs/bytecode/migrate.byte
```

### Known issues

- Children spread (`...children`) need to be fixed by hand
- Some children type declaration might need so tweaking
- `Style.{"foo": styles}` will error (`Style.({"foo": styles})` works)
