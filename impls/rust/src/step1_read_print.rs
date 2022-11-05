mod reader;
mod types;

use std::io::Write;

use anyhow::Result;
use reader::read_str;
use types::MalVal;

fn read(input: &str) -> Result<MalVal> {
    read_str(input)
}

fn eval(ast: &MalVal) -> &MalVal {
    ast
}

fn print(val: &MalVal) -> String {
    val.pr_str(true)
}

fn rep(input: &str) -> String {
    match read(input) {
        Ok(ast) => print(eval(&ast)),
        Err(e) => e.to_string(),
    }
}

fn main() {
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
            println!("{}", rep(input));
        }
        buffer.clear();
    }
}
