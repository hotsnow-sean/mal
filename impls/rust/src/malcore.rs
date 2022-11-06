use std::{cell::Cell, rc::Rc};

use anyhow::Result;

use crate::{read_str, MalVal};

pub const NS: [(&str, fn(&[Rc<MalVal>]) -> Result<Rc<MalVal>>); 24] = [
    ("+", add),
    ("-", sub),
    ("*", mul),
    ("/", div),
    ("prn", prn),
    ("list", list),
    ("list?", is_list),
    ("empty?", is_empty),
    ("count", count),
    ("=", eq),
    ("<", lt),
    ("<=", lte),
    (">", gt),
    (">=", gte),
    ("pr-str", pr_str),
    ("str", str),
    ("println", println),
    ("read-string", read_string),
    ("slurp", slurp),
    ("atom", atom),
    ("atom?", is_atom),
    ("deref", deref),
    ("reset!", reset),
    ("swap!", swap),
];

fn add(args: &[Rc<MalVal>]) -> Result<Rc<MalVal>> {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Integer(i), MalVal::Integer(j)) => Ok(Rc::new(MalVal::Integer(i + j))),
        _ => unreachable!(),
    }
}
fn sub(args: &[Rc<MalVal>]) -> Result<Rc<MalVal>> {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Integer(i), MalVal::Integer(j)) => Ok(Rc::new(MalVal::Integer(i - j))),
        _ => unreachable!(),
    }
}
fn mul(args: &[Rc<MalVal>]) -> Result<Rc<MalVal>> {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Integer(i), MalVal::Integer(j)) => Ok(Rc::new(MalVal::Integer(i * j))),
        _ => unreachable!(),
    }
}
fn div(args: &[Rc<MalVal>]) -> Result<Rc<MalVal>> {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Integer(i), MalVal::Integer(j)) => Ok(Rc::new(MalVal::Integer(i / j))),
        _ => unreachable!(),
    }
}

fn prn(args: &[Rc<MalVal>]) -> Result<Rc<MalVal>> {
    println!(
        "{}",
        args.iter()
            .map(|v| v.pr_str(true))
            .collect::<Vec<_>>()
            .join(" ")
    );
    Ok(Rc::new(MalVal::Nil))
}

fn list(args: &[Rc<MalVal>]) -> Result<Rc<MalVal>> {
    Ok(Rc::new(MalVal::List(args.to_vec())))
}

fn is_list(args: &[Rc<MalVal>]) -> Result<Rc<MalVal>> {
    match args[0].as_ref() {
        MalVal::List(_) => Ok(Rc::new(MalVal::Bool(true))),
        _ => Ok(Rc::new(MalVal::Bool(false))),
    }
}

fn is_empty(args: &[Rc<MalVal>]) -> Result<Rc<MalVal>> {
    match args[0].as_ref() {
        MalVal::List(list) | MalVal::Vector(list) => Ok(Rc::new(MalVal::Bool(list.is_empty()))),
        _ => unreachable!(),
    }
}

fn count(args: &[Rc<MalVal>]) -> Result<Rc<MalVal>> {
    match args[0].as_ref() {
        MalVal::List(list) | MalVal::Vector(list) => {
            Ok(Rc::new(MalVal::Integer(list.len().try_into().unwrap())))
        }
        _ => Ok(Rc::new(MalVal::Integer(0))),
    }
}

fn eq(args: &[Rc<MalVal>]) -> Result<Rc<MalVal>> {
    Ok(Rc::new(MalVal::Bool(args[0].as_ref() == args[1].as_ref())))
}

fn lt(args: &[Rc<MalVal>]) -> Result<Rc<MalVal>> {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Integer(i), MalVal::Integer(j)) => Ok(Rc::new(MalVal::Bool(i < j))),
        _ => unreachable!(),
    }
}
fn lte(args: &[Rc<MalVal>]) -> Result<Rc<MalVal>> {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Integer(i), MalVal::Integer(j)) => Ok(Rc::new(MalVal::Bool(i <= j))),
        _ => unreachable!(),
    }
}
fn gt(args: &[Rc<MalVal>]) -> Result<Rc<MalVal>> {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Integer(i), MalVal::Integer(j)) => Ok(Rc::new(MalVal::Bool(i > j))),
        _ => unreachable!(),
    }
}
fn gte(args: &[Rc<MalVal>]) -> Result<Rc<MalVal>> {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Integer(i), MalVal::Integer(j)) => Ok(Rc::new(MalVal::Bool(i >= j))),
        _ => unreachable!(),
    }
}

fn pr_str(args: &[Rc<MalVal>]) -> Result<Rc<MalVal>> {
    Ok(Rc::new(MalVal::String(
        args.iter()
            .map(|v| v.pr_str(true))
            .collect::<Vec<_>>()
            .join(" "),
    )))
}

fn str(args: &[Rc<MalVal>]) -> Result<Rc<MalVal>> {
    Ok(Rc::new(MalVal::String(
        args.iter()
            .map(|v| v.pr_str(false))
            .collect::<Vec<_>>()
            .join(""),
    )))
}

fn println(args: &[Rc<MalVal>]) -> Result<Rc<MalVal>> {
    println!(
        "{}",
        args.iter()
            .map(|v| v.pr_str(false))
            .collect::<Vec<_>>()
            .join(" ")
    );
    Ok(Rc::new(MalVal::Nil))
}

fn read_string(args: &[Rc<MalVal>]) -> Result<Rc<MalVal>> {
    match args[0].as_ref() {
        MalVal::String(string) => Ok(Rc::new(read_str(string.as_str())?)),
        _ => unreachable!(),
    }
}

fn slurp(args: &[Rc<MalVal>]) -> Result<Rc<MalVal>> {
    match args[0].as_ref() {
        MalVal::String(file) => Ok(Rc::new(MalVal::String(
            std::fs::read_to_string(file).unwrap(),
        ))),
        _ => unreachable!(),
    }
}

fn atom(args: &[Rc<MalVal>]) -> Result<Rc<MalVal>> {
    Ok(Rc::new(MalVal::Atom(Cell::new(args[0].clone()))))
}

fn is_atom(args: &[Rc<MalVal>]) -> Result<Rc<MalVal>> {
    match args[0].as_ref() {
        MalVal::Atom(_) => Ok(Rc::new(MalVal::Bool(true))),
        _ => Ok(Rc::new(MalVal::Bool(false))),
    }
}

fn deref(args: &[Rc<MalVal>]) -> Result<Rc<MalVal>> {
    match args[0].as_ref() {
        MalVal::Atom(v) => {
            let m = v.replace(Rc::new(MalVal::Nil));
            v.set(m.clone());
            Ok(m)
        }
        _ => unreachable!(),
    }
}

fn reset(args: &[Rc<MalVal>]) -> Result<Rc<MalVal>> {
    match args[0].as_ref() {
        MalVal::Atom(v) => {
            v.set(args[1].clone());
            Ok(args[1].clone())
        }
        _ => unreachable!(),
    }
}

fn swap(args: &[Rc<MalVal>]) -> Result<Rc<MalVal>> {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Atom(v), MalVal::Fn(func)) => {
            let mut params = vec![v.replace(Rc::new(MalVal::Nil))];
            params.append(&mut args[2..].to_vec());
            let result = func.run(&params)?;
            v.set(result.clone());
            Ok(result)
        }
        _ => unreachable!(),
    }
}
