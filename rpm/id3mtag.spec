Summary: Command line mass tagging utility for audio files
Name: id3mtag
Version: 0.80
Release: 1%{?dist}
License: BSD
Source: http://github.com/squell/id3/releases/download/%{version}/id3-%{version}.tar.gz
URL: https://squell.github.io/id3

# disable installation of bash-completion using --without completion
%bcond_without completion

%description
id3 mass tagger is a tool for manipulating id3, id3v2 and lyrics3 tags in
multiple files. It can generate tag fields from the filename and other 
variables, and/or rename files, using an intuitive syntax.

%prep
%setup -q -n id3-%{version}

%build
make %{?_smp_mflags} CFLAGS="%{optflags}"
strip -s id3

# makeinstall ("prefix=") is correct here, not make_install ("DESTDIR=")
%install
%makeinstall %{?with_completion:bash_completion}

%files
%defattr(-,root,root,-)
%{_bindir}/id3
%{_mandir}/man1/*
%{_datarootdir}/bash-completion/completions/id3
%doc README CHANGES COPYING

%changelog
* Sun Dec 20 2015 Marc Schoolderman <squell@alumina.nl> 0.80-1
- ID3v2.4 support (read/convert-to-ID3v2.3)
- Detailed listing of tags
- bash completion
* Thu Jan 29 2015 Marc Schoolderman <squell@alumina.nl> 0.79-1
- update to new version
