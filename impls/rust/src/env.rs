use std::{cell::RefCell, collections::HashMap, rc::Rc};

use crate::MalVal;

#[derive(Default)]
pub struct Env {
    outer: Option<Rc<RefCell<Env>>>,
    data: HashMap<String, Rc<MalVal>>,
}

impl Env {
    pub fn new(outer: Rc<RefCell<Env>>) -> Self {
        Self {
            outer: Some(outer),
            data: Default::default(),
        }
    }

    pub fn bind_expr(&mut self, binds: Vec<String>, exprs: Vec<Rc<MalVal>>) {
        let mut bind_iter = binds.into_iter();
        let mut expr_iter = exprs.into_iter().peekable();
        while let Some(k) = bind_iter.next() {
            match k.as_str() {
                "&" => {
                    let k = bind_iter.next().unwrap();
                    let v = Rc::new(MalVal::List(expr_iter.collect::<Vec<_>>()));
                    self.data.insert(k, v);
                    break;
                }
                _ => {
                    self.data.insert(k, expr_iter.next().unwrap());
                }
            }
        }
    }

    pub fn set(&mut self, symbol: String, value: Rc<MalVal>) {
        self.data.insert(symbol, value);
    }

    pub fn get(&self, symbol: &str) -> Option<Rc<MalVal>> {
        self.data.get(symbol).cloned().or_else(|| {
            self.outer
                .as_ref()
                .and_then(|o| o.as_ref().borrow().get(symbol))
        })
    }
}
