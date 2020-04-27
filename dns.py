from dns.resolver import dns
import dns.name

answers = dns.resolver.query('millersville.edu', 'A')
print(' query qname:', answers.qname, ' num ans.', len(answers))
for rdata in answers:
    print('IP for millersville.edu:', rdata.to_text())

answers3 = dns.resolver.query('millersville.edu', 'MX')
print(' query qname:', answers3.qname, ' num ans.', len(answers3))
for rdata3 in answers3:
    print('MX for millersville.edu:', rdata3.exchange.to_text())

answers4 = dns.resolver.query('millersville.edu', 'NS')
print(' query qname:', answers4.qname, ' num ans.', len(answers4))
for rdata4 in answers4:
    print('NS for millersville.edu:', rdata4.to_text())

answers5 = dns.resolver.query('millersville.edu', 'SOA')
print(' query qname:', answers5.qname, ' num ans.', len(answers5))
for rdata5 in answers5:
    print('SOA for millersville.edu:', rdata5.to_text())

answers6 = dns.resolver.query('millersville.edu', 'TXT')
print(' query qname:', answers6.qname, ' num ans.', len(answers6))
for rdata6 in answers6:
    print('TXT for millersville.edu:', rdata6.to_text())

###############################################################################

n = dns.name.from_text('ftp.millersville.edu')
o = dns.name.from_text('millersville.edu')
print('is ftp.millersville.edu a subdomain of millersville.edu', n.is_subdomain(o))         # True
print('is ftp.millersville.edu a superdomain of millersville.edu', n.is_superdomain(o))       # False
print(n > o)                     # True
rel = n.relativize(o)           # rel is the relative name 'ftp'
n2 = rel + o
print(n2 == n)                   # True
print(n.labels)                  # ('ftp', 'millersville', 'edu', '')
