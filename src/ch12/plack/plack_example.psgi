my $app = sub {
    return [200, ['Content-Type' => 'text/plain'], ["Test message from Plack.\n"]];
}
