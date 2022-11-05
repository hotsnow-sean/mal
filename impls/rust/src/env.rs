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
