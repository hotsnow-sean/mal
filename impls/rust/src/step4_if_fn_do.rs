mod env;
mod malcore;
mod reader;
mod types;

use std::{cell::RefCell, collections::HashMap, io::Write, rc::Rc};

use anyhow::{anyhow, Result};
use env::Env;
use malcore::NS;
use reader::read_str;
use types::MalVal;

fn read(input: &str) -> Result<MalVal> {
    read_str(input)
}

fn eval_ast(ast: Rc<MalVal>, env: Rc<RefCell<Env>>) -> Result<Rc<MalVal>> {
    match ast.as_ref() {
        MalVal::Symbol(symbol) => env
            .borrow()
            .get(symbol)
            .ok_or_else(|| anyhow!("'{symbol}' not found.")),
        MalVal::List(list) => {
            let mut buffer = Vec::new();
            for v in list {
                buffer.push(eval(v.clone(), env.clone())?);
            }
            Ok(Rc::new(MalVal::List(buffer)))
        }
        MalVal::Vector(vector) => {
            let mut buffer = Vec::new();
            for v in vector {
                buffer.push(eval(v.clone(), env.clone())?);
            }
            Ok(Rc::new(MalVal::Vector(buffer)))
        }
        MalVal::HashMap(hashmap) => {
            let mut buffer = HashMap::new();
            for (k, v) in hashmap {
                buffer.insert(k.clone(), eval(v.clone(), env.clone())?);
            }
            Ok(Rc::new(MalVal::HashMap(buffer)))
        }
        _ => Ok(ast),
    }
}

fn eval(ast: Rc<MalVal>, env: Rc<RefCell<Env>>) -> Result<Rc<MalVal>> {
    match ast.as_ref() {
        MalVal::List(list) if list.is_empty() => Ok(ast),
        MalVal::List(list) => {
            if let MalVal::Symbol(symbol) = list[0].as_ref() {
                match symbol.as_str() {
                    "def!" => match list[1].as_ref() {
                        MalVal::Symbol(symbol) => {
                            let v = eval(list[2].clone(), env.clone())?;
                            env.borrow_mut().set(symbol.clone(), v.clone());
                            return Ok(v);
                        }
                        _ => unreachable!(),
                    },
                    "let*" => {
                        let n_env = Rc::new(RefCell::new(Env::new(env)));
                        match list[1].as_ref() {
                            MalVal::List(binds) | MalVal::Vector(binds) => {
                                let mut iter = binds.iter();
                                while let Some(v) = iter.next() {
                                    match v.as_ref() {
                                        MalVal::Symbol(symbol) => {
                                            let value =
                                                eval(iter.next().unwrap().clone(), n_env.clone())?;
                                            n_env.borrow_mut().set(symbol.clone(), value);
                                        }
                                        _ => unreachable!(),
                                    }
                                }
                            }
                            _ => unreachable!(),
                        }
                        return eval(list[2].clone(), n_env);
                    }
                    "do" => {
                        let mut value = eval(list[1].clone(), env.clone())?;
                        for v in &list[2..] {
                            value = eval(v.clone(), env.clone())?;
                        }
                        return Ok(value);
                    }
                    "if" => {
                        let condition = eval(list[1].clone(), env.clone())?;
                        return match condition.as_ref() {
                            MalVal::Nil | MalVal::Bool(false) => {
                                if list.len() > 3 {
                                    eval(list[3].clone(), env)
                                } else {
                                    Ok(Rc::new(MalVal::Nil))
                                }
                            }
                            _ => eval(list[2].clone(), env),
                        };
                    }
                    "fn*" => {
                        let binds = match list[1].as_ref() {
                            MalVal::List(list) | MalVal::Vector(list) => list
                                .iter()
                                .map(|v| match v.as_ref() {
                                    MalVal::Symbol(symbol) => symbol.to_string(),
                                    _ => unreachable!(),
                                })
                                .collect::<Vec<_>>(),
                            _ => unreachable!(),
                        };
                        let body = list[2].clone();
                        let func = move |args: &[Rc<MalVal>]| -> Rc<MalVal> {
                            let mut n_env = Env::new(env.clone());
                            n_env.bind_expr(binds.clone(), args.to_owned());
                            eval(body.clone(), Rc::new(RefCell::new(n_env))).unwrap()
                        };
                        return Ok(Rc::new(MalVal::Fn(Rc::new(func))));
                    }
                    _ => (),
                }
            }
            let ast = eval_ast(ast, env)?;
            match ast.as_ref() {
                MalVal::List(list) => match list[0].as_ref() {
                    MalVal::Fn(func) => Ok((func.as_ref())(&list[1..])),
                    _ => unreachable!(),
                },
                _ => unreachable!(),
            }
        }
        _ => eval_ast(ast, env),
    }
}

fn print(val: &MalVal) -> String {
    val.pr_str(true)
}

fn rep(input: &str, env: &Rc<RefCell<Env>>) -> String {
    match read(input) {
        Ok(ast) => match eval(Rc::new(ast), env.clone()) {
            Ok(v) => print(v.as_ref()),
            Err(e) => e.to_string(),
        },
        Err(e) => e.to_string(),
    }
}

fn main() {
    let mut env = Env::default();
    for (k, v) in NS {
        env.set(k.to_string(), Rc::new(MalVal::Fn(Rc::new(v))));
    }
    let env = Rc::new(RefCell::new(env));
    rep("(def! not (fn* (a) (if a false true)))", &env);

    let mut buffer = String::new();
    loop {
        print!("user> ");
        std::io::stdout().flush().unwrap();
        let input = match std::io::stdin().read_line(&mut buffer) {
            Ok(0) => {
                println!();
                break;
            }
            Err(_) => break,
            Ok(_) => buffer.trim(),
        };
        if !input.is_empty() {
            println!("{}", rep(input, &env));
        }
        buffer.clear();
    }
}
