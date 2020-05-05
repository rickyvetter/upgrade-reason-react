// inline comment to check Reason versions

[@react.component]
let myComponent = () => {
  let myRef: React.Ref.t(React.element) = React.useRef(<div />);
  myRef->React.Ref.setCurrent(<span />);
  React.Ref.setCurrent(myRef, <p />);
  myRef->React.Ref.current === React.Ref.current(myRef);
};