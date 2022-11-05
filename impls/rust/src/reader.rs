use std::{collections::HashMap, iter::Peekable, rc::Rc, str::CharIndices};

use anyhow::{bail, Result};

use crate::types::MalVal;

struct Reader<'a> {
    source: &'a str,
    iter: Peekable<CharIndices<'a>>,
}

impl<'a> Reader<'a> {
    fn new(source: &'a str) -> Self {
        Self {
            source,
            iter: source.char_indices().peekable(),
        }
    }
}

impl<'a> Iterator for Reader<'a> {
    type Item = &'a str;

    fn next(&mut self) -> Option<Self::Item> {
        while self
            .iter
            .next_if(|(_, c)| c.is_whitespace() || c == &',')
            .is_some()
        {}
        if let Some((i, c)) = self.iter.next() {
            match c {
                '~' => {
                    self.iter.next_if(|(_, c)| c == &'@');
                }
                _ if "[]{}()'`~^@".contains(c) => (),
                '"' => {
                    while let Some((_, c)) = self.iter.next() {
                        if c == '\\' {
                            self.iter.next();
                        } else if c == '"' {
                            break;
                        }
                    }
                }
                ';' => while self.iter.next().is_some() {},
                _ if !"[]{}()'`~^@\";".contains(c) => {
                    while self
                        .iter
                        .next_if(|(_, c)| !c.is_whitespace() && !"[]{}()'`~^@\";,".contains(*c))
                        .is_some()
                    {}
                }
                _ => unreachable!(),
            }
            match self.iter.peek() {
                Some((j, _)) => Some(&self.source[i..*j]),
                None => Some(&self.source[i..]),
            }
        } else {
            None
        }
    }
}

pub fn read_str(input: &str) -> Result<MalVal> {
    let mut reader = Reader::new(input).peekable();
    read_form(&mut reader)
}

fn read_form(reader: &mut Peekable<Reader>) -> Result<MalVal> {
    match reader.next() {
        Some("(") => read_list(reader),
        Some("[") => read_vector(reader),
        Some("{") => read_hashmap(reader),
        None => unreachable!(),
        Some(s) => {
            let mut iter = s.chars().peekable();
            iter.next_if_eq(&'-');
            if let Some(n) = iter.next() {
                if n.is_ascii_digit() {
                    match s.parse::<i32>() {
                        Ok(i) => return Ok(MalVal::Integer(i)),
                        Err(_) => (),
                    }
                }
            }
            let first = s.chars().next().unwrap();
            if first == ':' {
                Ok(MalVal::Keyword(s[1..].to_string()))
            } else if first == '"' {
                Ok(MalVal::String(unescape(&s[1..])?))
            } else {
                match s {
                    "'" | "`" | "~" | "~@" | "@" => {
                        let prefix = match s {
                            "'" => "quote",
                            "`" => "quasiquote",
                            "~" => "unquote",
                            "~@" => "splice-unquote",
                            "@" => "deref",
                            _ => unreachable!(),
                        };
                        let v = read_form(reader)?;
                        Ok(MalVal::List(vec![
                            Rc::new(MalVal::Prefix(prefix)),
                            Rc::new(v),
                        ]))
                    }
                    "^" => {
                        let first = read_form(reader)?;
                        let second = read_form(reader)?;
                        Ok(MalVal::List(vec![
                            Rc::new(MalVal::Prefix("with-meta")),
                            Rc::new(second),
                            Rc::new(first),
                        ]))
                    }
                    _ => Ok(MalVal::Symbol(s.to_string())),
                }
            }
        }
    }
}

fn read_list(reader: &mut Peekable<Reader>) -> Result<MalVal> {
    let mut list = Vec::new();
    while let Some(&s) = reader.peek() {
        match s {
            ")" => {
                reader.next();
                return Ok(MalVal::List(list));
            }
            _ => list.push(Rc::new(read_form(reader)?)),
        }
    }
    bail!("EOF")
}

fn read_vector(reader: &mut Peekable<Reader>) -> Result<MalVal> {
    let mut vector = Vec::new();
    while let Some(&s) = reader.peek() {
        match s {
            "]" => {
                reader.next();
                return Ok(MalVal::Vector(vector));
            }
            _ => vector.push(Rc::new(read_form(reader)?)),
        }
    }
    bail!("EOF")
}

fn read_hashmap(reader: &mut Peekable<Reader>) -> Result<MalVal> {
    let mut hashmap = HashMap::new();
    while let Some(&s) = reader.peek() {
        match s {
            "}" => {
                reader.next();
                return Ok(MalVal::HashMap(hashmap));
            }
            _ => {
                let k = match read_form(reader)? {
                    MalVal::Keyword(s) => format!(":{s}"),
                    MalVal::String(s) => format!("\"{s}\""),
                    _ => unreachable!(),
                };
                let v = read_form(reader)?;
                hashmap.insert(k, Rc::new(v));
            }
        }
    }
    bail!("EOF")
}

fn unescape(s: &str) -> Result<String> {
    let mut buffer = String::with_capacity(s.len());
    let mut iter = s.chars().peekable();
    while let Some(c) = iter.next() {
        if c == '\\' {
            match iter.next() {
                Some('\\') => buffer.push('\\'),
                Some('n') => buffer.push('\n'),
                Some('"') => buffer.push('"'),
                _ => unreachable!(),
            }
        } else if c == '"' {
            println!("{buffer}");
            return Ok(buffer);
        } else {
            buffer.push(c);
        }
    }
    bail!("EOF")
}
