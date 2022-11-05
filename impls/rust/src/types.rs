use std::{collections::HashMap, rc::Rc};

pub enum MalVal {
    List(Vec<Rc<MalVal>>),
    Vector(Vec<Rc<MalVal>>),
    HashMap(HashMap<String, Rc<MalVal>>),
    Prefix(&'static str),
    Keyword(String),
    String(String),
    Integer(i32),
    Symbol(String),
}

impl MalVal {
    pub fn pr_str(&self, readably: bool) -> String {
        match self {
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
            MalVal::Prefix(prefix) => format!("{prefix}"),
            MalVal::Keyword(keyword) => format!(":{keyword}"),
            MalVal::String(string) => {
                if readably {
                    format!("{string:?}")
                } else {
                    format!("{string}")
                }
            }
            MalVal::Integer(int) => format!("{int}"),
            MalVal::Symbol(symbol) => format!("{symbol}"),
        }
    }
}
