const mineflayer = require('mineflayer')

const bot = mineflayer.createBot({
	host : 'localhost',
	port : 25565,
	username : 'bot',

})

bot.once('login', () => { bot.chat('Hello world!') })

bot.on('chat', (username, message) => {
	if (username === bot.username)
		return bot.chat(message)
})
