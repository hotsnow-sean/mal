use std::{cell::RefCell, collections::HashMap, io::Write, rc::Rc};

use rust2::{read_str, Env, MalError, MalFn, MalResult, MalVal};

fn read(input: &str) -> Result<MalVal, MalError> {
    read_str(input)
}

fn eval_ast(ast: Rc<MalVal>, env: Rc<RefCell<Env>>) -> MalResult {
    match ast.as_ref() {
        MalVal::Symbol(symbol) => env
            .borrow()
            .get(symbol)
            .ok_or_else(|| MalError::Other(format!("'{symbol}' not found."))),
        MalVal::List(list, _) => {
            let mut buffer = Vec::new();
            for v in list {
                buffer.push(eval(v.clone(), env.clone())?);
            }
            Ok(Rc::new(MalVal::List(buffer, None)))
        }
        MalVal::Vector(vector, _) => {
            let mut buffer = Vec::new();
            for v in vector {
                buffer.push(eval(v.clone(), env.clone())?);
            }
            Ok(Rc::new(MalVal::Vector(buffer, None)))
        }
        MalVal::HashMap(hashmap, _) => {
            let mut buffer = HashMap::new();
            for (k, v) in hashmap {
                buffer.insert(k.clone(), eval(v.clone(), env.clone())?);
            }
            Ok(Rc::new(MalVal::HashMap(buffer, None)))
        }
        _ => Ok(ast),
    }
}

fn eval(ast: Rc<MalVal>, env: Rc<RefCell<Env>>) -> MalResult {
    match ast.as_ref() {
        MalVal::List(list, _) if list.is_empty() => Ok(ast),
        MalVal::List(list, _) => {
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
                            MalVal::List(binds, _) | MalVal::Vector(binds, _) => {
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
                    _ => (),
                }
            }
            let ast = eval_ast(ast, env)?;
            match ast.as_ref() {
                MalVal::List(list, _) => match list[0].as_ref() {
                    MalVal::Fn(func, _) => match func.as_ref() {
                        MalFn::RegularFn(func) => (func)(&list[1..]),
                        _ => unreachable!(),
                    },
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

fn add(args: &[Rc<MalVal>]) -> MalResult {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Integer(i), MalVal::Integer(j)) => Ok(Rc::new(MalVal::Integer(i + j))),
        _ => unreachable!(),
    }
}
fn sub(args: &[Rc<MalVal>]) -> MalResult {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Integer(i), MalVal::Integer(j)) => Ok(Rc::new(MalVal::Integer(i - j))),
        _ => unreachable!(),
    }
}
fn mul(args: &[Rc<MalVal>]) -> MalResult {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Integer(i), MalVal::Integer(j)) => Ok(Rc::new(MalVal::Integer(i * j))),
        _ => unreachable!(),
    }
}
fn div(args: &[Rc<MalVal>]) -> MalResult {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Integer(i), MalVal::Integer(j)) => Ok(Rc::new(MalVal::Integer(i / j))),
        _ => unreachable!(),
    }
}

fn main() {
    let mut env = Env::default();
    env.set(
        "+".to_string(),
        Rc::new(MalVal::Fn(Rc::new(MalFn::RegularFn(Rc::new(add))), None)),
    );
    env.set(
        "-".to_string(),
        Rc::new(MalVal::Fn(Rc::new(MalFn::RegularFn(Rc::new(sub))), None)),
    );
    env.set(
        "*".to_string(),
        Rc::new(MalVal::Fn(Rc::new(MalFn::RegularFn(Rc::new(mul))), None)),
    );
    env.set(
        "/".to_string(),
        Rc::new(MalVal::Fn(Rc::new(MalFn::RegularFn(Rc::new(div))), None)),
    );
    let env = Rc::new(RefCell::new(env));

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
