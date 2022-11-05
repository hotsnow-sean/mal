use std::{collections::HashMap, fmt::Display, rc::Rc};

pub type MalFunc = dyn Fn(&[Rc<MalVal>]) -> Rc<MalVal>;

pub enum MalVal {
    Fn(Rc<MalFunc>),
    List(Vec<Rc<MalVal>>),
    Vector(Vec<Rc<MalVal>>),
    HashMap(HashMap<String, Rc<MalVal>>),
    Prefix(&'static str),
    Keyword(String),
    String(String),
    Integer(i32),
    Symbol(String),
}

impl Display for MalVal {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.pr_str(true))
    }
}

impl MalVal {
    pub fn pr_str(&self, readably: bool) -> String {
        match self {
            MalVal::Fn(_) => "#<function>".to_string(),
            MalVal::List(list) => {
                format!(
                    "({})",
                    list.iter()
                        .map(|v| v.as_ref().pr_str(readably))
                        .collect::<Vec<_>>()
                        .join(" ")
                )
            }
            MalVal::Vector(vector) => {
                format!(
                    "[{}]",
                    vector
                        .iter()
                        .map(|v| v.as_ref().pr_str(readably))
                        .collect::<Vec<_>>()
                        .join(" ")
                )
            }
            MalVal::HashMap(map) => {
                format!(
                    "{{{}}}",
                    map.iter()
                        .map(|(k, v)| format!("{k} {}", v.as_ref().pr_str(readably)))
                        .collect::<Vec<_>>()
                        .join(" ")
                )
            }
            MalVal::Prefix(prefix) => prefix.to_string(),
            MalVal::Keyword(keyword) => format!(":{keyword}"),
            MalVal::String(string) => {
                if readably {
                    format!("{string:?}")
                } else {
                    string.to_string()
                }
            }
            MalVal::Integer(int) => format!("{int}"),
            MalVal::Symbol(symbol) => symbol.to_string(),
        }
    }
}
