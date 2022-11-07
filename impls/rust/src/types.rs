use std::{
    cell::{Cell, RefCell},
    collections::HashMap,
    fmt::{Debug, Display},
    rc::Rc,
};

use thiserror::Error;

use crate::Env;

pub type MalResult = Result<Rc<MalVal>, MalError>;

#[derive(Error, Debug)]
pub enum MalError {
    #[error("Eexception {0}")]
    Throw(Rc<MalVal>),

    #[error("EOF")]
    Unbalance(&'static str),

    #[error("no more token, need continue")]
    Continue,

    #[error("{0}")]
    Other(String),
}

pub struct MalFunc {
    pub ast: Rc<MalVal>,
    pub params: Vec<String>,
    pub env: Rc<RefCell<Env>>,
    pub func: fn(Rc<MalVal>, Rc<RefCell<Env>>) -> MalResult,
    pub is_marco: bool,
}

pub enum MalFn {
    MalFunc(MalFunc),
    RegularFn(Rc<dyn Fn(&[Rc<MalVal>]) -> MalResult>),
}

#[derive(PartialEq, Eq, Hash, Clone)]
pub enum Hashable {
    Keyword(String),
    String(String),
}

pub enum MalVal {
    Fn(Rc<MalFn>, Option<Rc<MalVal>>),
    List(Vec<Rc<MalVal>>, Option<Rc<MalVal>>),
    Vector(Vec<Rc<MalVal>>, Option<Rc<MalVal>>),
    HashMap(HashMap<Hashable, Rc<MalVal>>, Option<Rc<MalVal>>),
    Keyword(String),
    String(String),
    Integer(i64),
    Bool(bool),
    Nil,
    Symbol(String),
    Atom(Cell<Rc<MalVal>>),
}

impl MalFunc {
    pub fn construct_marco(&self) -> Self {
        MalFunc {
            ast: self.ast.clone(),
            params: self.params.to_vec(),
            env: self.env.clone(),
            func: self.func,
            is_marco: true,
        }
    }

    pub fn run(&self, args: &[Rc<MalVal>]) -> MalResult {
        let mut n_env = Env::new(self.env.clone());
        n_env.bind_expr(self.params.clone(), args.to_vec());
        (self.func)(self.ast.clone(), Rc::new(RefCell::new(n_env)))
    }
}

impl MalFn {
    pub fn custom_func(
        ast: Rc<MalVal>,
        params: Vec<String>,
        env: Rc<RefCell<Env>>,
        func: fn(Rc<MalVal>, Rc<RefCell<Env>>) -> MalResult,
    ) -> Self {
        Self::MalFunc(MalFunc {
            ast,
            params,
            env,
            func,
            is_marco: false,
        })
    }

    pub fn run(&self, args: &[Rc<MalVal>]) -> MalResult {
        match self {
            MalFn::RegularFn(func) => (func)(args),
            MalFn::MalFunc(func) => func.run(args),
        }
    }
}

impl Display for MalVal {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.pr_str(true))
    }
}

impl PartialEq for MalVal {
    fn eq(&self, other: &Self) -> bool {
        match (self, other) {
            (Self::List(l0, _), Self::List(r0, _)) => l0 == r0,
            (Self::List(l0, _), Self::Vector(r0, _)) => l0 == r0,
            (Self::Vector(l0, _), Self::List(r0, _)) => l0 == r0,
            (Self::Vector(l0, _), Self::Vector(r0, _)) => l0 == r0,
            (Self::HashMap(l0, _), Self::HashMap(r0, _)) => l0 == r0,
            (Self::Keyword(l0), Self::Keyword(r0)) => l0 == r0,
            (Self::String(l0), Self::String(r0)) => l0 == r0,
            (Self::Integer(l0), Self::Integer(r0)) => l0 == r0,
            (Self::Bool(l0), Self::Bool(r0)) => l0 == r0,
            (Self::Symbol(l0), Self::Symbol(r0)) => l0 == r0,
            _ => core::mem::discriminant(self) == core::mem::discriminant(other),
        }
    }
}

impl Hashable {
    pub fn pr_str(&self, readably: bool) -> String {
        match self {
            Hashable::Keyword(keyword) => format!(":{keyword}"),
            Hashable::String(string) => {
                if readably {
                    format!("{string:?}")
                } else {
                    string.to_string()
                }
            }
        }
    }
}

impl From<&MalVal> for Hashable {
    fn from(v: &MalVal) -> Self {
        match v {
            MalVal::Keyword(s) => Hashable::Keyword(s.to_string()),
            MalVal::String(s) => Hashable::String(s.to_string()),
            _ => panic!("cannot hash"),
        }
    }
}

impl From<&Hashable> for MalVal {
    fn from(v: &Hashable) -> Self {
        match v {
            Hashable::Keyword(s) => MalVal::Keyword(s.to_string()),
            Hashable::String(s) => MalVal::String(s.to_string()),
        }
    }
}

impl From<MalError> for Rc<MalVal> {
    fn from(e: MalError) -> Self {
        match e {
            MalError::Throw(v) => v,
            MalError::Continue => Rc::new(MalVal::Nil),
            _ => Rc::new(MalVal::String(e.to_string())),
        }
    }
}

impl MalVal {
    pub fn pr_str(&self, readably: bool) -> String {
        match self {
            MalVal::Fn(..) => "#<function>".to_string(),
            MalVal::List(list, _) => {
                format!(
                    "({})",
                    list.iter()
                        .map(|v| v.as_ref().pr_str(readably))
                        .collect::<Vec<_>>()
                        .join(" ")
                )
            }
            MalVal::Vector(vector, _) => {
                format!(
                    "[{}]",
                    vector
                        .iter()
                        .map(|v| v.as_ref().pr_str(readably))
                        .collect::<Vec<_>>()
                        .join(" ")
                )
            }
            MalVal::HashMap(map, _) => {
                format!(
                    "{{{}}}",
                    map.iter()
                        .map(|(k, v)| format!(
                            "{} {}",
                            k.pr_str(readably),
                            v.as_ref().pr_str(readably)
                        ))
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

impl Debug for MalVal {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::List(arg0, _) => f.debug_tuple("List").field(arg0).finish(),
            Self::Vector(arg0, _) => f.debug_tuple("Vector").field(arg0).finish(),
            Self::Keyword(arg0) => f.debug_tuple("Keyword").field(arg0).finish(),
            Self::String(arg0) => f.debug_tuple("String").field(arg0).finish(),
            Self::Integer(arg0) => f.debug_tuple("Integer").field(arg0).finish(),
            Self::Bool(arg0) => f.debug_tuple("Bool").field(arg0).finish(),
            Self::Nil => write!(f, "Nil"),
            Self::Symbol(arg0) => f.debug_tuple("Symbol").field(arg0).finish(),
            _ => write!(f, "{}", self.pr_str(true)),
        }
    }
}
