const Libp2p = require('libp2p')
const TCP = require('libp2p-tcp')
const { NOISE } = require('libp2p-noise')
const MPLEX = require('libp2p-mplex')


const node = new Libp2p(
{
    modules:
    {
        transport: [ TCP ],
        connEncryption: [ NOISE ],
        streamMuxer: [ MPLEX ]
    }
})

node.start((err) =>
{
    if (err)
    {
        console.error(err)
        process.exit(1)
    }
    console.log('Node is running!')
})
