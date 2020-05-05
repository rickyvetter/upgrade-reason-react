## ReasonReact migration script

For migrating from 0.7 to 0.8. Running attempts to find React.ref usage and convert to the new record shape. It cannot find all React.ref usage - particularly if the code opens modules. In this case you might have to manually convert some code.

### Usage

Upgrading from reason-react 0.7.0 to 0.8.0: `npm install https://github.com/rickyvetter/upgrade-reason-react\#0.7.0-to-0.8.0`

```console
find src -name "*.re" | upgrade-reason-react
```
