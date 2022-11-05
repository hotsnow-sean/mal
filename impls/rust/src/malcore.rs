use std::rc::Rc;

use crate::MalVal;

pub const NS: [(&str, fn(&[Rc<MalVal>]) -> Rc<MalVal>); 17] = [
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
];

fn add(args: &[Rc<MalVal>]) -> Rc<MalVal> {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Integer(i), MalVal::Integer(j)) => Rc::new(MalVal::Integer(i + j)),
        _ => unreachable!(),
    }
}
fn sub(args: &[Rc<MalVal>]) -> Rc<MalVal> {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Integer(i), MalVal::Integer(j)) => Rc::new(MalVal::Integer(i - j)),
        _ => unreachable!(),
    }
}
fn mul(args: &[Rc<MalVal>]) -> Rc<MalVal> {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Integer(i), MalVal::Integer(j)) => Rc::new(MalVal::Integer(i * j)),
        _ => unreachable!(),
    }
}
fn div(args: &[Rc<MalVal>]) -> Rc<MalVal> {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Integer(i), MalVal::Integer(j)) => Rc::new(MalVal::Integer(i / j)),
        _ => unreachable!(),
    }
}

fn prn(args: &[Rc<MalVal>]) -> Rc<MalVal> {
    println!(
        "{}",
        args.iter()
            .map(|v| v.pr_str(true))
            .collect::<Vec<_>>()
            .join(" ")
    );
    Rc::new(MalVal::Nil)
}

fn list(args: &[Rc<MalVal>]) -> Rc<MalVal> {
    Rc::new(MalVal::List(args.to_vec()))
}

fn is_list(args: &[Rc<MalVal>]) -> Rc<MalVal> {
    match args[0].as_ref() {
        MalVal::List(_) => Rc::new(MalVal::Bool(true)),
        _ => Rc::new(MalVal::Bool(false)),
    }
}

fn is_empty(args: &[Rc<MalVal>]) -> Rc<MalVal> {
    match args[0].as_ref() {
        MalVal::List(list) | MalVal::Vector(list) => Rc::new(MalVal::Bool(list.is_empty())),
        _ => unreachable!(),
    }
}

fn count(args: &[Rc<MalVal>]) -> Rc<MalVal> {
    match args[0].as_ref() {
        MalVal::List(list) | MalVal::Vector(list) => {
            Rc::new(MalVal::Integer(list.len().try_into().unwrap()))
        }
        _ => Rc::new(MalVal::Integer(0)),
    }
}

fn eq(args: &[Rc<MalVal>]) -> Rc<MalVal> {
    Rc::new(MalVal::Bool(args[0].as_ref() == args[1].as_ref()))
}

fn lt(args: &[Rc<MalVal>]) -> Rc<MalVal> {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Integer(i), MalVal::Integer(j)) => Rc::new(MalVal::Bool(i < j)),
        _ => unreachable!(),
    }
}
fn lte(args: &[Rc<MalVal>]) -> Rc<MalVal> {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Integer(i), MalVal::Integer(j)) => Rc::new(MalVal::Bool(i <= j)),
        _ => unreachable!(),
    }
}
fn gt(args: &[Rc<MalVal>]) -> Rc<MalVal> {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Integer(i), MalVal::Integer(j)) => Rc::new(MalVal::Bool(i > j)),
        _ => unreachable!(),
    }
}
fn gte(args: &[Rc<MalVal>]) -> Rc<MalVal> {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Integer(i), MalVal::Integer(j)) => Rc::new(MalVal::Bool(i >= j)),
        _ => unreachable!(),
    }
}

fn pr_str(args: &[Rc<MalVal>]) -> Rc<MalVal> {
    Rc::new(MalVal::String(
        args.iter()
            .map(|v| v.pr_str(true))
            .collect::<Vec<_>>()
            .join(" "),
    ))
}

fn str(args: &[Rc<MalVal>]) -> Rc<MalVal> {
    Rc::new(MalVal::String(
        args.iter()
            .map(|v| v.pr_str(false))
            .collect::<Vec<_>>()
            .join(""),
    ))
}

fn println(args: &[Rc<MalVal>]) -> Rc<MalVal> {
    println!(
        "{}",
        args.iter()
            .map(|v| v.pr_str(false))
            .collect::<Vec<_>>()
            .join(" ")
    );
    Rc::new(MalVal::Nil)
}
