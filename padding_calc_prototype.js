#!/bin/node

let payload_length = parseInt(process.argv[2]);
let cipher_block_size  = parseInt(process.argv[3]);

//console.log(((8<block_size)?block_size:8)-((5+data_length)%((8<block_size)?block_size:8)));

console.group(`SOLVE "padding_length == ( ( 8 < cipher_block_size ) ? cipher_block_size : 8 ) - ( ( 1 + 4 + payload_length ) % ( ( 8 < cipher_block_size ) ? cipher_block_size : 8 ) )"`);
	console.group(`SOLVE "( ( 8 < cipher_block_size ) ? cipher_block_size : 8 ) - ( ( 1 + 4 + payload_length ) % ( ( 8 < cipher_block_size ) ? cipher_block_size : 8 ) )"`);
		console.group(`SOLVE "( ( 8 < cipher_block_size ) ? cipher_block_size : 8 )"`)
			console.group(`SOLVE "( 8 < cipher_block_size ) ? cipher_block_size : 8"`);
				console.group(`SOLVE "( 8 < cipher_block_size )"`);
					console.group(`SOLVE "8 < cipher_block_size)"`);
						console.group(`SOLVE "cipher_block_size"`);
							console.group(`GET "cipher_block_size"`);
								console.group(`RETURN "${cipher_block_size}"`);

								console.groupEnd();
							console.groupEnd();
							console.group(`RETURN "${cipher_block_size}"`);
								
							console.groupEnd();
						console.groupEnd();
						console.group(`SOLVE "8 < ${cipher_block_size}"`)
							console.group(`RETURN "${8 < cipher_block_size}"`);

							console.groupEnd();
						console.groupEnd();
					console.groupEnd();
				console.groupEnd();
			console.groupEnd();
		console.groupEnd();
	console.groupEnd();
console.groupEnd();
