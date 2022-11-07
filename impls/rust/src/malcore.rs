use std::{cell::Cell, collections::HashMap, io::Write, rc::Rc};

use crate::{read_str, MalError, MalResult, MalVal};

pub const NS: [(&str, fn(&[Rc<MalVal>]) -> MalResult); 61] = [
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
    ("cons", cons),
    ("concat", concat),
    ("vec", vec),
    ("nth", nth),
    ("first", first),
    ("rest", rest),
    ("throw", throw),
    ("apply", apply),
    ("map", map),
    ("nil?", is_nil),
    ("true?", is_true),
    ("false?", is_false),
    ("symbol?", is_symbol),
    ("symbol", symbol),
    ("keyword", keyword),
    ("keyword?", is_keyword),
    ("vector", vector),
    ("vector?", is_vector),
    ("sequential?", is_sequential),
    ("hash-map", hash_map),
    ("map?", is_map),
    ("assoc", assoc),
    ("dissoc", dissoc),
    ("get", get),
    ("contains?", is_contains),
    ("keys", keys),
    ("vals", vals),
    ("readline", readline),
    ("time-ms", time_ms),
    ("meta", meta),
    ("with-meta", with_meta),
    ("fn?", is_fn),
    ("string?", is_string),
    ("number?", is_number),
    ("seq", seq),
    ("conj", conj),
    ("macro?", is_macro),
];

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

fn prn(args: &[Rc<MalVal>]) -> MalResult {
    println!(
        "{}",
        args.iter()
            .map(|v| v.pr_str(true))
            .collect::<Vec<_>>()
            .join(" ")
    );
    Ok(Rc::new(MalVal::Nil))
}

fn list(args: &[Rc<MalVal>]) -> MalResult {
    Ok(Rc::new(MalVal::List(args.to_vec(), None)))
}

fn is_list(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::List(..) => Ok(Rc::new(MalVal::Bool(true))),
        _ => Ok(Rc::new(MalVal::Bool(false))),
    }
}

fn is_empty(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::List(list, _) | MalVal::Vector(list, _) => {
            Ok(Rc::new(MalVal::Bool(list.is_empty())))
        }
        _ => unreachable!(),
    }
}

fn count(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::List(list, _) | MalVal::Vector(list, _) => {
            Ok(Rc::new(MalVal::Integer(list.len().try_into().unwrap())))
        }
        _ => Ok(Rc::new(MalVal::Integer(0))),
    }
}

fn eq(args: &[Rc<MalVal>]) -> MalResult {
    Ok(Rc::new(MalVal::Bool(args[0].as_ref() == args[1].as_ref())))
}

fn lt(args: &[Rc<MalVal>]) -> MalResult {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Integer(i), MalVal::Integer(j)) => Ok(Rc::new(MalVal::Bool(i < j))),
        _ => unreachable!(),
    }
}
fn lte(args: &[Rc<MalVal>]) -> MalResult {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Integer(i), MalVal::Integer(j)) => Ok(Rc::new(MalVal::Bool(i <= j))),
        _ => unreachable!(),
    }
}
fn gt(args: &[Rc<MalVal>]) -> MalResult {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Integer(i), MalVal::Integer(j)) => Ok(Rc::new(MalVal::Bool(i > j))),
        _ => unreachable!(),
    }
}
fn gte(args: &[Rc<MalVal>]) -> MalResult {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Integer(i), MalVal::Integer(j)) => Ok(Rc::new(MalVal::Bool(i >= j))),
        _ => unreachable!(),
    }
}

fn pr_str(args: &[Rc<MalVal>]) -> MalResult {
    Ok(Rc::new(MalVal::String(
        args.iter()
            .map(|v| v.pr_str(true))
            .collect::<Vec<_>>()
            .join(" "),
    )))
}

fn str(args: &[Rc<MalVal>]) -> MalResult {
    Ok(Rc::new(MalVal::String(
        args.iter()
            .map(|v| v.pr_str(false))
            .collect::<Vec<_>>()
            .join(""),
    )))
}

fn println(args: &[Rc<MalVal>]) -> MalResult {
    println!(
        "{}",
        args.iter()
            .map(|v| v.pr_str(false))
            .collect::<Vec<_>>()
            .join(" ")
    );
    Ok(Rc::new(MalVal::Nil))
}

fn read_string(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::String(string) => Ok(Rc::new(read_str(string.as_str())?)),
        _ => unreachable!(),
    }
}

fn slurp(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::String(file) => Ok(Rc::new(MalVal::String(
            std::fs::read_to_string(file).unwrap(),
        ))),
        _ => unreachable!(),
    }
}

fn atom(args: &[Rc<MalVal>]) -> MalResult {
    Ok(Rc::new(MalVal::Atom(Cell::new(args[0].clone()))))
}

fn is_atom(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::Atom(_) => Ok(Rc::new(MalVal::Bool(true))),
        _ => Ok(Rc::new(MalVal::Bool(false))),
    }
}

fn deref(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::Atom(v) => {
            let m = v.replace(Rc::new(MalVal::Nil));
            v.set(m.clone());
            Ok(m)
        }
        _ => unreachable!(),
    }
}

fn reset(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::Atom(v) => {
            v.set(args[1].clone());
            Ok(args[1].clone())
        }
        _ => unreachable!(),
    }
}

fn swap(args: &[Rc<MalVal>]) -> MalResult {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Atom(v), MalVal::Fn(func, _)) => {
            let mut params = vec![v.replace(Rc::new(MalVal::Nil))];
            params.append(&mut args[2..].to_vec());
            let result = func.run(&params)?;
            v.set(result.clone());
            Ok(result)
        }
        _ => unreachable!(),
    }
}

fn cons(args: &[Rc<MalVal>]) -> MalResult {
    match args[1].as_ref() {
        MalVal::List(list, _) | MalVal::Vector(list, _) => {
            let mut buffer = vec![args[0].clone()];
            buffer.append(&mut list.to_vec());
            Ok(Rc::new(MalVal::List(buffer, None)))
        }
        _ => unreachable!(),
    }
}

fn concat(args: &[Rc<MalVal>]) -> MalResult {
    let mut buffer = Vec::new();
    for v in args {
        match v.as_ref() {
            MalVal::List(list, _) | MalVal::Vector(list, _) => {
                buffer.append(&mut list.to_vec());
            }
            _ => unreachable!(),
        }
    }
    Ok(Rc::new(MalVal::List(buffer, None)))
}

fn vec(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::List(list, _) => Ok(Rc::new(MalVal::Vector(list.to_vec(), None))),
        MalVal::Vector(..) => Ok(args[0].clone()),
        _ => unreachable!(),
    }
}

fn nth(args: &[Rc<MalVal>]) -> MalResult {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::List(list, _), MalVal::Integer(i))
        | (MalVal::Vector(list, _), MalVal::Integer(i)) => list
            .get(*i as usize)
            .cloned()
            .ok_or_else(|| MalError::Throw(Rc::new(MalVal::String("out of bounds".to_string())))),
        _ => unreachable!(),
    }
}

fn first(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::List(list, _) | MalVal::Vector(list, _) => Ok(list
            .first()
            .map_or_else(|| Rc::new(MalVal::Nil), |v| v.clone())),
        MalVal::Nil => Ok(Rc::new(MalVal::Nil)),
        _ => unreachable!(),
    }
}

fn rest(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::List(list, _) | MalVal::Vector(list, _) => {
            let mut iter = list.iter();
            iter.next();
            Ok(Rc::new(MalVal::List(iter.cloned().collect(), None)))
        }
        MalVal::Nil => Ok(Rc::new(MalVal::List(Vec::new(), None))),
        _ => unreachable!(),
    }
}

fn throw(args: &[Rc<MalVal>]) -> MalResult {
    Err(MalError::Throw(args[0].clone()))
}

fn apply(args: &[Rc<MalVal>]) -> MalResult {
    match (args.first(), args.last()) {
        (Some(f), Some(l)) => match (f.as_ref(), l.as_ref()) {
            (MalVal::Fn(f, _), MalVal::List(l, _)) | (MalVal::Fn(f, _), MalVal::Vector(l, _)) => {
                let mut buffer = args[1..args.len() - 1].to_vec();
                buffer.append(&mut l.to_vec());
                f.run(&buffer)
            }
            _ => unreachable!(),
        },
        _ => unreachable!(),
    }
}

fn map(args: &[Rc<MalVal>]) -> MalResult {
    match (args[0].as_ref(), args[1].as_ref()) {
        (MalVal::Fn(f, _), MalVal::List(l, _)) | (MalVal::Fn(f, _), MalVal::Vector(l, _)) => {
            let mut buffer = Vec::with_capacity(l.len());
            for v in l {
                buffer.push(f.run(&[v.clone()])?);
            }
            Ok(Rc::new(MalVal::List(buffer, None)))
        }
        _ => unreachable!(),
    }
}

fn is_nil(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::Nil => Ok(Rc::new(MalVal::Bool(true))),
        _ => Ok(Rc::new(MalVal::Bool(false))),
    }
}

fn is_true(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::Bool(true) => Ok(Rc::new(MalVal::Bool(true))),
        _ => Ok(Rc::new(MalVal::Bool(false))),
    }
}

fn is_false(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::Bool(false) => Ok(Rc::new(MalVal::Bool(true))),
        _ => Ok(Rc::new(MalVal::Bool(false))),
    }
}

fn is_symbol(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::Symbol(_) => Ok(Rc::new(MalVal::Bool(true))),
        _ => Ok(Rc::new(MalVal::Bool(false))),
    }
}

fn symbol(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::String(s) => Ok(Rc::new(MalVal::Symbol(s.clone()))),
        _ => unreachable!(),
    }
}

fn keyword(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::String(s) => Ok(Rc::new(MalVal::Keyword(s.clone()))),
        MalVal::Keyword(_) => Ok(args[0].clone()),
        _ => unreachable!(),
    }
}

fn is_keyword(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::Keyword(_) => Ok(Rc::new(MalVal::Bool(true))),
        _ => Ok(Rc::new(MalVal::Bool(false))),
    }
}

fn vector(args: &[Rc<MalVal>]) -> MalResult {
    Ok(Rc::new(MalVal::Vector(args.to_vec(), None)))
}

fn is_vector(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::Vector(..) => Ok(Rc::new(MalVal::Bool(true))),
        _ => Ok(Rc::new(MalVal::Bool(false))),
    }
}

fn is_sequential(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::Vector(..) | MalVal::List(..) => Ok(Rc::new(MalVal::Bool(true))),
        _ => Ok(Rc::new(MalVal::Bool(false))),
    }
}

fn hash_map(args: &[Rc<MalVal>]) -> MalResult {
    let mut hashmap = HashMap::new();
    let mut iter = args.iter();
    while let (Some(k), Some(v)) = (iter.next(), iter.next()) {
        let k = k.as_ref().into();
        hashmap.insert(k, v.clone());
    }
    Ok(Rc::new(MalVal::HashMap(hashmap, None)))
}

fn is_map(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::HashMap(..) => Ok(Rc::new(MalVal::Bool(true))),
        _ => Ok(Rc::new(MalVal::Bool(false))),
    }
}

fn assoc(args: &[Rc<MalVal>]) -> MalResult {
    let mut iter = args.iter();
    let mut hashmap = match iter.next() {
        Some(h) => match h.as_ref() {
            MalVal::HashMap(h, _) => h.clone(),
            _ => unreachable!(),
        },
        _ => unreachable!(),
    };
    while let (Some(k), Some(v)) = (iter.next(), iter.next()) {
        let k = k.as_ref().into();
        hashmap.insert(k, v.clone());
    }
    Ok(Rc::new(MalVal::HashMap(hashmap, None)))
}

fn dissoc(args: &[Rc<MalVal>]) -> MalResult {
    let mut iter = args.iter();
    let mut hashmap = match iter.next() {
        Some(h) => match h.as_ref() {
            MalVal::HashMap(h, _) => h.clone(),
            _ => unreachable!(),
        },
        _ => unreachable!(),
    };
    for k in iter {
        let k = k.as_ref().into();
        hashmap.remove(&k);
    }
    Ok(Rc::new(MalVal::HashMap(hashmap, None)))
}

fn get(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::HashMap(h, _) => {
            let k = args[1].as_ref().into();
            Ok(h.get(&k)
                .map_or_else(|| Rc::new(MalVal::Nil), |v| v.clone()))
        }
        MalVal::Nil => Ok(args[0].clone()),
        _ => unreachable!(),
    }
}

fn is_contains(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::HashMap(h, _) => {
            let k = args[1].as_ref().into();
            Ok(Rc::new(MalVal::Bool(h.contains_key(&k))))
        }
        _ => unreachable!(),
    }
}

fn keys(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::HashMap(h, _) => Ok(Rc::new(MalVal::List(
            h.keys().map(|k| Rc::new(k.into())).collect(),
            None,
        ))),
        _ => unreachable!(),
    }
}

fn vals(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::HashMap(h, _) => Ok(Rc::new(MalVal::List(h.values().cloned().collect(), None))),
        _ => unreachable!(),
    }
}

fn readline(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::String(s) => {
            let mut buffer = String::new();
            print!("{s}");
            std::io::stdout().flush().unwrap();
            match std::io::stdin().read_line(&mut buffer) {
                Ok(n) if n > 0 => {
                    buffer.pop();
                    Ok(Rc::new(MalVal::String(buffer)))
                }
                _ => Ok(Rc::new(MalVal::Nil)),
            }
        }
        _ => unreachable!(),
    }
}

fn meta(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::List(_, data)
        | MalVal::Vector(_, data)
        | MalVal::HashMap(_, data)
        | MalVal::Fn(_, data) => data
            .as_ref()
            .cloned()
            .map_or_else(|| Ok(Rc::new(MalVal::Nil)), Ok),
        _ => unreachable!(),
    }
}

fn with_meta(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::List(list, _) => Ok(Rc::new(MalVal::List(list.to_vec(), Some(args[1].clone())))),
        MalVal::Vector(vector, _) => Ok(Rc::new(MalVal::Vector(
            vector.to_vec(),
            Some(args[1].clone()),
        ))),
        MalVal::HashMap(hashmap, _) => Ok(Rc::new(MalVal::HashMap(
            hashmap.clone(),
            Some(args[1].clone()),
        ))),
        MalVal::Fn(func, _) => Ok(Rc::new(MalVal::Fn(func.clone(), Some(args[1].clone())))),
        _ => unreachable!(),
    }
}

fn time_ms(_: &[Rc<MalVal>]) -> MalResult {
    Ok(Rc::new(MalVal::Integer(
        std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .unwrap()
            .as_millis()
            .try_into()
            .unwrap(),
    )))
}

fn conj(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::List(list, data) => {
            let mut buffer = args[1..].iter().rev().cloned().collect::<Vec<_>>();
            buffer.append(&mut list.to_vec());
            Ok(Rc::new(MalVal::List(buffer, data.clone())))
        }
        MalVal::Vector(vector, data) => {
            let mut buffer = vector.to_vec();
            buffer.append(&mut args[1..].to_vec());
            Ok(Rc::new(MalVal::Vector(buffer, data.clone())))
        }
        _ => unreachable!(),
    }
}

fn is_string(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::String(..) => Ok(Rc::new(MalVal::Bool(true))),
        _ => Ok(Rc::new(MalVal::Bool(false))),
    }
}

fn is_number(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::Integer(..) => Ok(Rc::new(MalVal::Bool(true))),
        _ => Ok(Rc::new(MalVal::Bool(false))),
    }
}

fn is_fn(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::Fn(f, _) => match f.as_ref() {
            crate::MalFn::MalFunc(f) if f.is_marco => Ok(Rc::new(MalVal::Bool(false))),
            _ => Ok(Rc::new(MalVal::Bool(true))),
        },
        _ => Ok(Rc::new(MalVal::Bool(false))),
    }
}

fn is_macro(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::Fn(f, _) => match f.as_ref() {
            crate::MalFn::MalFunc(f) if f.is_marco => Ok(Rc::new(MalVal::Bool(true))),
            _ => Ok(Rc::new(MalVal::Bool(false))),
        },
        _ => Ok(Rc::new(MalVal::Bool(false))),
    }
}

fn seq(args: &[Rc<MalVal>]) -> MalResult {
    match args[0].as_ref() {
        MalVal::List(list, _) => {
            if list.is_empty() {
                Ok(Rc::new(MalVal::Nil))
            } else {
                Ok(args[0].clone())
            }
        }
        MalVal::Vector(vector, data) => {
            if vector.is_empty() {
                Ok(Rc::new(MalVal::Nil))
            } else {
                Ok(Rc::new(MalVal::List(vector.to_vec(), data.clone())))
            }
        }
        MalVal::String(string) => {
            if string.is_empty() {
                Ok(Rc::new(MalVal::Nil))
            } else {
                Ok(Rc::new(MalVal::List(
                    string
                        .chars()
                        .map(|c| Rc::new(MalVal::String(c.to_string())))
                        .collect(),
                    None,
                )))
            }
        }
        MalVal::Nil => Ok(args[0].clone()),
        _ => unreachable!(),
    }
}
