## ReasonReact migration script

For migrating from 0.7 to 0.8. Running attempts to find React.ref usage and convert to the new record shape. It cannot find all React.ref usage - particularly if the code opens modules. In this case you might have to manually convert some code.

### Usage

```console
find src -name "*.re" | ./node_modules/upgrade-reason-react/lib/bs/bytecode/migrate.byte
```
