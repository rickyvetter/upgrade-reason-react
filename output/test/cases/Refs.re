[@react.component]
let myComponent = () => {
  let myRef: React.ref(React.element) = React.useRef(<div />);
  myRef.React.current = <span />;
  myRef.React.current = <p />;

  myRef.React.current === myRef.React.current;
};
