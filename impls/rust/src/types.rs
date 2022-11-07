use std::{
    cell::{Cell, RefCell},
    collections::HashMap,
    fmt::Display,
    rc::Rc,
};

use anyhow::Result;

use crate::Env;

pub struct MalFunc {
    pub ast: Rc<MalVal>,
    pub params: Vec<String>,
    pub env: Rc<RefCell<Env>>,
    pub func: fn(Rc<MalVal>, Rc<RefCell<Env>>) -> Result<Rc<MalVal>>,
}

pub enum MalFn {
    MalFunc(MalFunc),
    RegularFn(Rc<dyn Fn(&[Rc<MalVal>]) -> Result<Rc<MalVal>>>),
}

impl MalFn {
    pub fn custom_func(
        ast: Rc<MalVal>,
        params: Vec<String>,
        env: Rc<RefCell<Env>>,
        func: fn(Rc<MalVal>, Rc<RefCell<Env>>) -> Result<Rc<MalVal>>,
    ) -> Self {
        Self::MalFunc(MalFunc {
            ast,
            params,
            env,
            func,
        })
    }

    pub fn run(&self, args: &[Rc<MalVal>]) -> Result<Rc<MalVal>> {
        match self {
            MalFn::RegularFn(func) => (func)(args),
            MalFn::MalFunc(func) => {
                let mut n_env = Env::new(func.env.clone());
                n_env.bind_expr(func.params.clone(), args.to_vec());
                (func.func)(func.ast.clone(), Rc::new(RefCell::new(n_env)))
            }
        }
    }
}

pub enum MalVal {
    Fn(Rc<MalFn>),
    List(Vec<Rc<MalVal>>),
    Vector(Vec<Rc<MalVal>>),
    HashMap(HashMap<String, Rc<MalVal>>),
    Keyword(String),
    String(String),
    Integer(i64),
    Bool(bool),
    Nil,
    Symbol(String),
    Atom(Cell<Rc<MalVal>>),
}

impl Display for MalVal {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.pr_str(true))
    }
}

impl PartialEq for MalVal {
    fn eq(&self, other: &Self) -> bool {
        match (self, other) {
            (Self::List(l0), Self::List(r0)) => l0 == r0,
            (Self::List(l0), Self::Vector(r0)) => l0 == r0,
            (Self::Vector(l0), Self::List(r0)) => l0 == r0,
            (Self::Vector(l0), Self::Vector(r0)) => l0 == r0,
            (Self::HashMap(l0), Self::HashMap(r0)) => l0 == r0,
            (Self::Keyword(l0), Self::Keyword(r0)) => l0 == r0,
            (Self::String(l0), Self::String(r0)) => l0 == r0,
            (Self::Integer(l0), Self::Integer(r0)) => l0 == r0,
            (Self::Bool(l0), Self::Bool(r0)) => l0 == r0,
            (Self::Symbol(l0), Self::Symbol(r0)) => l0 == r0,
            _ => core::mem::discriminant(self) == core::mem::discriminant(other),
        }
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
            MalVal::Bool(b) => format!("{b}"),
            MalVal::Nil => "nil".to_string(),
            MalVal::Atom(v) => {
                let m = v.replace(Rc::new(MalVal::Nil));
                v.set(m.clone());
                format!("(atom {})", m.as_ref().pr_str(readably))
            }
        }
    }
}
